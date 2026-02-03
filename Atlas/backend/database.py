"""
Database setup for Atlas.

This module:
- Creates the SQLAlchemy engine (using SQLite by default)
- Creates a session factory (SessionLocal)
- Defines the Base class for ORM models to inherit from
- Provides a get_db() dependency for FastAPI routes
"""

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker, declarative_base
import os

# You can override this with an environment variable if you ever deploy
DATABASE_URL = os.getenv("ATLAS_DATABASE_URL", "sqlite:///./atlas.db")

# For SQLite, we need this connect_args; for other DBs it's not required
if DATABASE_URL.startswith("sqlite"):
    engine = create_engine(DATABASE_URL, connect_args={"check_same_thread": False})
else:
    engine = create_engine(DATABASE_URL)

# Session factory
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# Base class for ORM models (User, Entry, etc.)
Base = declarative_base()


def get_db():
    """FastAPI dependency that yields a database session and closes it after the request."""
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


def run_migrations() -> None:
    """Run Alembic migrations to upgrade database to latest schema.
    Falls back to create_all if Alembic is not installed (e.g. fresh venv).
    """
    import os
    import subprocess
    import sys

    try:
        backend_dir = os.path.dirname(os.path.abspath(__file__))
        result = subprocess.run(
            [sys.executable, "-m", "alembic", "upgrade", "head"],
            cwd=backend_dir,
            env={**os.environ, "PYTHONPATH": os.path.dirname(backend_dir)},
            capture_output=True,
            text=True,
        )
        if result.returncode == 0:
            return
        # If alembic not found, fall through to create_all
        if "No module named alembic" not in (result.stderr or ""):
            raise RuntimeError(f"Migrations failed: {result.stderr or result.stdout}")
    except FileNotFoundError:
        pass

    # Fallback: create tables from models (for fresh installs without alembic)
    from models import Base, Category
    from seed_categories import DEFAULT_CATEGORIES

    Base.metadata.create_all(bind=engine)
    # Seed default categories if empty
    db = SessionLocal()
    try:
        if db.query(Category).count() == 0:
            for i, (name, slug, sort_order) in enumerate(DEFAULT_CATEGORIES, 1):
                db.add(Category(id=i, name=name, slug=slug, sort_order=sort_order))
            db.commit()
    finally:
        db.close()