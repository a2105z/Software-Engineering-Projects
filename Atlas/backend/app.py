"""
Atlas API – FastAPI application for academic journey tracking.

This module defines the HTTP API layer: routes, request/response validation,
and orchestration. Database logic lives in the repositories/ layer.
"""

from datetime import date
import csv
import hashlib
import io
from typing import List, Optional, Literal

from fastapi import FastAPI, Depends, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import StreamingResponse, JSONResponse
from sqlalchemy.orm import Session

from database import get_db, run_migrations
from models import Entry
import schemas
from repositories import UserRepository, EntryRepository, ReportRepository

# --------------------------------------------------
# Startup: run database migrations
# --------------------------------------------------

run_migrations()

app = FastAPI(
    title="Atlas Backend",
    description="API for tracking academic achievements and enrichment activities",
)

# --------------------------------------------------
# CORS: allow React dev server
# --------------------------------------------------

# Allow any localhost port (Vite may use 5173, 5174, 5175, etc.)
app.add_middleware(
    CORSMiddleware,
    allow_origin_regex=r"https?://(localhost|127\.0\.0\.1)(:[0-9]+)?$",
    allow_methods=["*"],
    allow_headers=["*"],
    allow_credentials=True,
)


# --------------------------------------------------
# Helpers
# --------------------------------------------------


def hash_password(raw: str) -> str:
    """
    Simple SHA-256 hash for demo purposes.
    In production, use passlib/bcrypt with salt.
    """
    return hashlib.sha256(raw.encode("utf-8")).hexdigest()


def _entry_to_out(entry: Entry) -> schemas.EntryOut:
    """Convert Entry ORM to EntryOut schema (includes category name from relationship)."""
    return schemas.EntryOut(
        id=entry.id,
        user_id=entry.user_id,
        category=entry.category_rel.name if entry.category_rel else "",
        date=entry.date,
        details=entry.details,
        created_at=entry.created_at,
    )


# --------------------------------------------------
# Root
# --------------------------------------------------


@app.get("/", tags=["Health"])
def root():
    """Health check; confirms the API is running."""
    return {"message": "Atlas backend is running 🚀"}


# --------------------------------------------------
# User + Auth
# --------------------------------------------------


@app.post("/users/", response_model=schemas.UserOut, tags=["Users"])
def create_user(user_in: schemas.UserCreate, db: Session = Depends(get_db)):
    """
    Sign up: create a new user with profile and password.
    Returns the created user (without password).
    """
    existing = UserRepository.get_by_username(db, user_in.username)
    if existing:
        raise HTTPException(status_code=400, detail="Username already taken")

    user = UserRepository.create(
        db,
        username=user_in.username,
        first_name=user_in.first_name,
        last_name=user_in.last_name,
        birthday=user_in.birthday,
        password_hash=hash_password(user_in.password),
    )
    return user


@app.post("/auth/login", response_model=schemas.UserOut, tags=["Auth"])
def login(user_in: schemas.UserLogin, db: Session = Depends(get_db)):
    """
    Log in: validate username and password, return user profile.
    """
    user = UserRepository.get_by_username(db, user_in.username)
    if not user or user.password_hash != hash_password(user_in.password):
        raise HTTPException(status_code=401, detail="Invalid username or password")
    return user


@app.get("/users/{username}", response_model=schemas.UserOut, tags=["Users"])
def get_user(username: str, db: Session = Depends(get_db)):
    """Fetch a user by username. Useful for debugging or scripts."""
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    return user


# --------------------------------------------------
# Categories & Entries
# --------------------------------------------------


@app.get("/users/{username}/categories", response_model=List[str], tags=["Entries"])
def get_user_categories(username: str, db: Session = Depends(get_db)):
    """
    Return all available categories. Validates that the user exists.
    """
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    return ReportRepository.get_categories(db)


@app.get("/users/{username}/entries", response_model=schemas.EntryListResponse, tags=["Entries"])
def list_entries(
    username: str,
    category: Optional[str] = None,
    search: Optional[str] = None,
    from_date: Optional[date] = Query(None, alias="from"),
    to_date: Optional[date] = Query(None, alias="to"),
    sort: Literal["date", "category", "created_at"] = "date",
    order: Literal["asc", "desc"] = "asc",
    limit: Optional[int] = Query(None, ge=1, le=1000),
    offset: int = Query(0, ge=0),
    db: Session = Depends(get_db),
):
    """
    List entries for a user with optional filtering, sorting, and pagination.
    Query params: category, search, from, to, sort, order, limit, offset.
    """
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    entries, total_count = EntryRepository.list_entries(
        db,
        user.id,
        category=category,
        search=search,
        from_date=from_date,
        to_date=to_date,
        sort=sort,
        order=order,
        limit=limit,
        offset=offset,
    )
    return schemas.EntryListResponse(
        entries=[_entry_to_out(e) for e in entries],
        total_count=total_count,
    )


@app.get("/users/{username}/entries/{entry_id}", response_model=schemas.EntryOut, tags=["Entries"])
def get_entry(
    username: str,
    entry_id: int,
    db: Session = Depends(get_db),
):
    """Fetch a single entry by ID for a user."""
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    entry = EntryRepository.get_by_id(db, user.id, entry_id)
    if not entry:
        raise HTTPException(status_code=404, detail="Entry not found")
    return _entry_to_out(entry)


@app.post("/users/{username}/entries", response_model=schemas.EntryOut, tags=["Entries"])
def create_entry_for_user(
    username: str,
    entry_in: schemas.EntryCreate,
    db: Session = Depends(get_db),
):
    """
    Create a new entry for a user.
    Category must match a category name in the categories table.
    """
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    category = ReportRepository.get_category_by_name(db, entry_in.category)
    if not category:
        raise HTTPException(
            status_code=400,
            detail=f"Invalid category: '{entry_in.category}'. Use a valid category from the categories list.",
        )

    entry = EntryRepository.create(
        db,
        user_id=user.id,
        category_id=category.id,
        date_val=entry_in.date,
        details=entry_in.details,
    )
    return _entry_to_out(entry)


@app.patch("/users/{username}/entries/{entry_id}", response_model=schemas.EntryOut, tags=["Entries"])
def update_entry_for_user(
    username: str,
    entry_id: int,
    entry_in: schemas.EntryUpdate,
    db: Session = Depends(get_db),
):
    """
    Update an entry (partial update).
    Only provided fields are changed.
    """
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    entry = EntryRepository.get_by_id(db, user.id, entry_id)
    if not entry:
        raise HTTPException(status_code=404, detail="Entry not found")

    category_id = None
    if entry_in.category is not None:
        category = ReportRepository.get_category_by_name(db, entry_in.category)
        if not category:
            raise HTTPException(
                status_code=400,
                detail=f"Invalid category: '{entry_in.category}'.",
            )
        category_id = category.id

    entry = EntryRepository.update(
        db,
        entry,
        category_id=category_id,
        date_val=entry_in.date,
        details=entry_in.details,
    )
    return _entry_to_out(entry)


@app.delete("/users/{username}/entries/{entry_id}", tags=["Entries"])
def delete_entry_for_user(
    username: str,
    entry_id: int,
    db: Session = Depends(get_db),
):
    """Delete a single entry by ID for a user."""
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    entry = EntryRepository.get_by_id(db, user.id, entry_id)
    if not entry:
        raise HTTPException(status_code=404, detail="Entry not found")

    EntryRepository.delete(db, entry)
    return {"detail": "Entry deleted"}


# --------------------------------------------------
# Analytics
# --------------------------------------------------


@app.get("/analytics/{username}/summary", response_model=schemas.AnalyticsSummary, tags=["Analytics"])
def analytics_summary(username: str, db: Session = Depends(get_db)):
    """
    Simple analytics summary: total entries, per-category counts,
    first and last entry dates.
    """
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    data = ReportRepository.analytics_summary(db, user.id)
    return schemas.AnalyticsSummary(username=username, **data)


# --------------------------------------------------
# Reports (SQL-based)
# --------------------------------------------------


@app.get("/reports/{username}/timeline", response_model=schemas.TimelineReport, tags=["Reports"])
def report_timeline(
    username: str,
    group_by: Literal["month", "quarter"] = "month",
    db: Session = Depends(get_db),
):
    """
    Entries grouped by month or quarter.
    Use group_by=month for YYYY-MM, group_by=quarter for YYYY-Qn.
    """
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    rows = ReportRepository.timeline_report(db, user.id, group_by=group_by)
    groups = [schemas.TimelineGroup(period=p, count=c) for p, c in rows]
    return schemas.TimelineReport(username=username, group_by=group_by, groups=groups)


@app.get("/reports/{username}/by-category", response_model=schemas.ByCategoryReport, tags=["Reports"])
def report_by_category(username: str, db: Session = Depends(get_db)):
    """Entry counts per category for a user."""
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    by_category = ReportRepository.by_category_report(db, user.id)
    return schemas.ByCategoryReport(username=username, by_category=by_category)


@app.get("/reports/{username}/activity-summary", response_model=schemas.ActivitySummaryReport, tags=["Reports"])
def report_activity_summary(username: str, db: Session = Depends(get_db)):
    """
    Rich activity summary: total entries, date range, per-category counts,
    and timeline by month.
    """
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    data = ReportRepository.activity_summary_report(db, user.id)
    timeline = [schemas.TimelineGroup(period=t["period"], count=t["count"]) for t in data["timeline_by_month"]]
    return schemas.ActivitySummaryReport(
        username=username,
        total_entries=data["total_entries"],
        first_entry_date=data["first_entry_date"],
        last_entry_date=data["last_entry_date"],
        entries_per_category=data["entries_per_category"],
        timeline_by_month=timeline,
    )


# --------------------------------------------------
# Export
# --------------------------------------------------


@app.get("/users/{username}/export", tags=["Export"])
def export_user_data(
    username: str,
    format: Literal["csv", "json"] = Query("json", alias="format"),
    db: Session = Depends(get_db),
):
    """
    Export a user's entries as CSV or JSON.
    CSV returns a downloadable file; JSON returns inline JSON.
    """
    user = UserRepository.get_by_username(db, username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    rows = EntryRepository.export_entries(db, user.id)

    if format == "json":
        data = [
            {
                "id": e.id,
                "category": cat_name,
                "date": e.date.isoformat(),
                "details": e.details,
                "created_at": e.created_at.isoformat(),
            }
            for e, cat_name in rows
        ]
        return JSONResponse(content={"username": username, "entries": data})

    # CSV
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(["category", "date", "details", "created_at"])
    for e, cat_name in rows:
        writer.writerow([
            cat_name,
            e.date.isoformat() if e.date else "",
            e.details or "",
            e.created_at.isoformat() if e.created_at else "",
        ])
    output.seek(0)
    return StreamingResponse(
        iter([output.getvalue()]),
        media_type="text/csv",
        headers={"Content-Disposition": f"attachment; filename=atlas_export_{username}.csv"},
    )


# --------------------------------------------------
# Main
# --------------------------------------------------


if __name__ == "__main__":
    import uvicorn

    uvicorn.run("app:app", host="0.0.0.0", port=8001, reload=True)
