# Atlas

**Atlas** is a full-stack web app that helps high school students (or undergrads) **track their academic journey** and organize achievements. It's a modern reimagining of *ScholarPath*, focused on database design, SQL, and clean backend architecture.

> **Folder name:** If your project folder is still named `ArcTrack`, rename it to `Atlas` for consistency. From the parent folder, run: `powershell -ExecutionPolicy Bypass -File rename_arcTrack_to_atlas.ps1` (after closing Cursor). You may need to recreate the backend venv after renaming.

It's built as a software-engineering project:

* 🐍 **Python / FastAPI backend** with SQLAlchemy and SQLite
* 📊 **Database-backed analytics** (summary stats, aggregation queries)
* 💻 **React frontend** with a clean UI
* 💾 Legacy **Tkinter desktop app** preserved under `legacy/` for comparison

You can use this both as:

* A personal academic tracker, and
* A portfolio project demonstrating backend, frontend, and database integration.

---

## ✨ Features

### Core user features

* 👤 **Accounts with profiles**

  * Create an account with **first name, last name, birthday, username, password**
  * Log in with **username + password**
  * Each user sees only their own entries and analytics

* 📂 **Academic & enrichment categories** (matching the original ScholarPath)

  * Awards & Honors
  * Professional Experiences
  * Summer Programs
  * SAT/ACT/SATII/ACTII Scores
  * AP Scores
  * GPA
  * Future Plans & Competitions
  * Personal Goals
  * Reflection Journal

* ➕ **Add entries per category**

  * Each entry has:

    * Date (via date picker)
    * Category
    * Details (text)
  * **AP Scores** gets special handling:

    * Separate *AP Exam* and *Score* fields
    * Stored as `"AP Biology - 5"` in the backend, like the Tkinter version

* ✏️ **Edit entries**

  * Update category, date, or details for existing entries

* 🔍 **Filter & search**

  * Filter entries by category
  * Date range filtering (from/to)
  * Free-text search in entry details
  * Sort by date, category, or created_at (asc/desc)
  * Pagination with limit and offset

* ❌ **Delete entries**

  * Remove entries from the database with a single click
  * UI confirms before deleting

* 📊 **Dashboard summary**

  * Total entries
  * First entry date
  * Most recent entry date
  * Count of entries per category

* 📈 **SQL-based reports**

  * Timeline report (entries grouped by month or quarter)
  * By-category report
  * Activity summary (combined view with timeline)

* 📤 **Data export**

  * Export your entries as CSV or JSON via API

---

## 🧠 Technical Overview

### Tech stack

**Backend**

* Python 3.10+
* FastAPI
* SQLAlchemy + SQLite (for persistent storage)
* Alembic (database migrations)
* Pydantic (request/response models)
* Uvicorn (ASGI server)

**Frontend**

* React (with Vite)
* Plain CSS (via `styles.css`)
* Fetch-based API wrapper in `src/api.js`

**Legacy**

* Tkinter desktop app, preserved under `legacy/`
* Local JSON storage (`tracker_data.json`)

---

## 🧱 Architecture

Project structure (simplified):

```txt
Atlas/
├── README.md
├── .gitignore
│
├── backend/
│   ├── requirements.txt
│   ├── app.py            # FastAPI app (API routes, CORS)
│   ├── database.py       # SQLAlchemy engine, migrations runner
│   ├── models.py         # User, Category, Entry ORM models
│   ├── schemas.py        # Pydantic models (UserCreate, EntryCreate, etc.)
│   ├── seed_categories.py # Default category definitions
│   ├── repositories/     # Database access layer
│   │   ├── user_repository.py
│   │   ├── entry_repository.py
│   │   └── report_repository.py
│   ├── migrations/       # Alembic migration scripts
│   └── scripts/          # Data scripts (import, export, backup)
│
├── frontend/
│   ├── package.json
│   ├── vite.config.*     # Vite config
│   ├── index.html
│   └── src/
│       ├── main.jsx
│       ├── App.jsx
│       ├── api.js
│       ├── pages/
│       │   └── LoginPage.jsx
│       ├── components/
│       │   ├── Navbar.jsx
│       │   ├── EntriesTable.jsx
│       │   ├── EntriesFilters.jsx
│       │   ├── EntriesPagination.jsx
│       │   └── EntryForm.jsx
│       └── styles/
│           └── styles.css
│
└── legacy/
    ├── desktop_tracker.py  # Original Tkinter ScholarPath
    └── tracker_data.json   # Legacy data file (if present)
```

### Database schema

**Main tables**

| Table     | Description                                      |
|-----------|--------------------------------------------------|
| `users`   | User accounts (username, profile, password hash) |
| `categories` | Lookup table for entry categories (name, slug, sort_order) |
| `entries` | Academic entries (user_id, category_id, date, details) |

**Relationships**

* `entries.user_id` → `users.id` (ON DELETE CASCADE)
* `entries.category_id` → `categories.id` (ON DELETE RESTRICT)

**User**

| Column        | Type     | Notes                              |
|---------------|----------|------------------------------------|
| id            | int      | Primary key                        |
| username      | str      | Unique, indexed                    |
| first_name    | str      |                                    |
| last_name     | str      |                                    |
| birthday      | date     |                                    |
| password_hash | str      | SHA-256 for demo                   |
| created_at    | datetime |                                    |
| updated_at    | datetime |                                    |

**Category**

| Column     | Type | Notes       |
|------------|------|-------------|
| id         | int  | Primary key |
| name       | str  | Unique      |
| slug       | str  | Unique      |
| sort_order | int  | Display order |

**Entry**

| Column      | Type     | Notes                        |
|-------------|----------|------------------------------|
| id          | int      | Primary key                  |
| user_id     | int      | FK → users.id, CASCADE       |
| category_id | int      | FK → categories.id, RESTRICT |
| date        | date     |                              |
| details     | text     |                              |
| created_at  | datetime |                              |
| updated_at  | datetime |                              |

### Migrations

Migrations run **automatically** on app startup via `database.run_migrations()`.

To run manually from the `backend/` directory:

```bash
python -m alembic upgrade head
```

To create a new migration after changing models:

```bash
python -m alembic revision --autogenerate -m "description"
```

### API endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | Health check |
| POST | `/users/` | Create user (sign-up) |
| POST | `/auth/login` | Log in |
| GET | `/users/{username}` | Get user by username |
| GET | `/users/{username}/categories` | List categories |
| GET | `/users/{username}/entries` | List entries (filter, sort, paginate) |
| GET | `/users/{username}/entries/{id}` | Get single entry |
| POST | `/users/{username}/entries` | Create entry |
| PATCH | `/users/{username}/entries/{id}` | Update entry |
| DELETE | `/users/{username}/entries/{id}` | Delete entry |
| GET | `/users/{username}/export?format=csv\|json` | Export entries |
| GET | `/analytics/{username}/summary` | Analytics summary |
| GET | `/reports/{username}/timeline?group_by=month\|quarter` | Timeline report |
| GET | `/reports/{username}/by-category` | Counts per category |
| GET | `/reports/{username}/activity-summary` | Full activity summary |

Interactive docs: `http://localhost:8000/docs`

---

## 🚀 Getting Started

### 1. Prerequisites

* **Python** 3.10+ (3.11 recommended)
* **Node.js** 18+ and npm
* Git (optional but recommended)

---

### 2. Backend setup (FastAPI + DB)

From the project root:

```bash
cd backend
python -m venv venv
```

Activate the virtual environment (PowerShell on Windows):

```bash
.\venv\Scripts\Activate.ps1
```

Install dependencies:

```bash
pip install -r requirements.txt
```

Run the backend:

```bash
python app.py
```

You should see:

```text
Uvicorn running on http://0.0.0.0:8001
```

Visit these in your browser:

* API root: `http://localhost:8000/`
* API docs: `http://localhost:8000/docs`

> 💡 Schema changes require new Alembic migrations. See the [Migrations](#migrations) section.

---

### 3. Frontend setup (React + Vite)

Open a **second terminal** for the frontend:

```bash
cd frontend
npm install
npm run dev
```

You should see something like:

```text
VITE vX.X.X  ready in XXX ms
  ➜  Local:   http://localhost:5173/
```

Open the app:

👉 `http://localhost:5173/`

The frontend is configured to talk to the backend at `http://localhost:8001` (or via proxy in dev) via `API_BASE_URL` in `src/api.js`.

---

## 🧭 Usage Walkthrough

1. **Create an account**

   * On the landing page, fill in:

     * First name, last name, birthday
     * Username
     * Password
   * Click **Create account**
   * You'll see a message prompting you to log in.

2. **Log in**

   * Enter your username and password in the login section.
   * Click **Log in**.
   * You'll be taken to the **Dashboard**.

3. **Dashboard**

   * See total entries, first/most recent dates, entries per category, and activity by month.
   * Use **Export** (JSON/CSV) to download your data.

4. **Entries page**

   * Switch via the navbar to **Entries**.
   * Filter by category, date range, and search. Sort and paginate as needed.
   * Add a new entry: select category, pick a date, enter details (or AP Exam + Score for AP Scores).
   * **Edit** or **Delete** entries via the action buttons.

5. **Log out**

   * Use the **Logout** button in the navbar to clear the session and return to the login screen.

---

## 🕰 Legacy: ScholarPath Desktop App

The original Tkinter-based **ScholarPath** app is preserved under `legacy/`:

* `legacy/desktop_tracker.py` – Tkinter GUI code
* `legacy/tracker_data.json` – JSON data file (if you've used it before)

You can still run it (from `legacy/`):

```bash
python desktop_tracker.py
```

This is useful for:

* Showing how the project evolved from a **desktop app** to a **full web stack**
* Using `import_legacy_data.py` to migrate old JSON into the new database.

**Importing legacy data:** From the `backend/` directory, run:

```bash
python scripts/import_legacy_data.py
```

Legacy users are created with default profile fields. They can log in with the password `changeme` (see the script output for details).

**Backing up the database:** From the `backend/` directory:

```bash
python scripts/backup_db.py
```

Creates a timestamped copy in `backend/backups/`. Or specify a path: `python scripts/backup_db.py /path/to/backup.db`

---

## 🔮 Possible Extensions

* **Robust auth**: Proper password hashing (`passlib`/`bcrypt`), JWT sessions
* **Charts**: Time-series visualizations for the dashboard
* **Change password**: Allow users to update their password after first login
