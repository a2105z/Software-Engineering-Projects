// frontend/src/App.jsx

import React, { useState, useEffect } from "react";

import Navbar from "./components/Navbar";
import LoginPage from "./pages/LoginPage";
import EntriesFilters from "./components/EntriesFilters";
import EntriesTable from "./components/EntriesTable";
import EntriesPagination from "./components/EntriesPagination";
import EntryForm from "./components/EntryForm";

import {
  API_BASE_URL,
  registerUser,
  loginUser,
  getActivitySummaryReport,
  listEntries,
  getCategories,
  createEntry,
  updateEntry,
  deleteEntry,
  exportUserData,
} from "./api";

// Default categories, matching your old Tkinter app
const DEFAULT_CATEGORIES = [
  "Awards & Honors",
  "Professional Experiences",
  "Summer Programs",
  "SAT/ACT/SATII/ACTII Scores",
  "AP Scores",
  "GPA",
  "Future Plans & Competitions",
  "Personal Goals",
  "Reflection Journal",
];

function App() {
  const [view, setView] = useState("login"); // 'login' | 'dashboard' | 'entries'

  const [user, setUser] = useState(null);
  const [error, setError] = useState("");
  const [authMessage, setAuthMessage] = useState("");

  const [loadingAuth, setLoadingAuth] = useState(false);

  // Dashboard / summary
  const [summary, setSummary] = useState(null);
  const [loadingSummary, setLoadingSummary] = useState(false);

  // Entries
  const [entries, setEntries] = useState([]);
  const [totalCount, setTotalCount] = useState(0);
  const [entriesLoading, setEntriesLoading] = useState(false);
  const [entryFilters, setEntryFilters] = useState({
    category: undefined,
    search: undefined,
    from: undefined,
    to: undefined,
    sort: "date",
    order: "desc",
    limit: 50,
    offset: 0,
  });
  const [editingEntryId, setEditingEntryId] = useState(null);
  const [exporting, setExporting] = useState(false);
  const [exportSuccess, setExportSuccess] = useState(false);

  // Categories: start with defaults, then override from backend if available
  const [categories, setCategories] = useState(DEFAULT_CATEGORIES);

  // Load categories when user logs in
  useEffect(() => {
    if (!user) return;
    async function loadCategoriesForUser() {
      try {
        const cats = await getCategories(user.username);
        // If backend returns none (new user), fall back to defaults
        setCategories(cats.length ? cats : DEFAULT_CATEGORIES);
      } catch (err) {
        console.error(err);
        // On error, keep defaults so UI still works
        setCategories(DEFAULT_CATEGORIES);
      }
    }
    loadCategoriesForUser();
  }, [user]);

  // ------------------------
  // Auth handlers
  // ------------------------

  async function handleLoginExisting({ username, password }) {
    setError("");
    setAuthMessage("");

    if (!username || !password) {
      setError("Please enter username and password.");
      return;
    }

    setLoadingAuth(true);
    try {
      const userData = await loginUser(username, password);
      setUser(userData);
      setView("dashboard");
      await loadSummaryForUser(userData.username);
    } catch (err) {
      console.error(err);
      setError(err.message || "Login failed.");
    } finally {
      setLoadingAuth(false);
    }
  }

  async function handleCreateAccount({
    firstName,
    lastName,
    birthday,
    username,
    password,
  }) {
    setError("");
    setAuthMessage("");

    if (!firstName || !lastName || !birthday || !username || !password) {
      setError("Please fill in all fields for account creation.");
      return;
    }

    setLoadingAuth(true);
    try {
      await registerUser({
        first_name: firstName,
        last_name: lastName,
        birthday,
        username,
        password,
      });

      setAuthMessage(
        "Account created! Now log in with your username and password."
      );
    } catch (err) {
      console.error(err);
      setError(err.message || "Failed to create account.");
    } finally {
      setLoadingAuth(false);
    }
  }

  function handleLogout() {
    setUser(null);
    setSummary(null);
    setEntries([]);
    setView("login");
    setError("");
    setAuthMessage("");
    setEntryFilters({
      category: undefined,
      search: undefined,
      from: undefined,
      to: undefined,
      sort: "date",
      order: "desc",
      limit: 50,
      offset: 0,
    });
    setEditingEntryId(null);
    // reset categories to defaults for next user
    setCategories(DEFAULT_CATEGORIES);
  }

  // ------------------------
  // Summary / analytics
  // ------------------------

  async function loadSummaryForUser(username) {
    setLoadingSummary(true);
    setError("");

    try {
      const data = await getActivitySummaryReport(username);
      setSummary(data);
    } catch (err) {
      console.error(err);
      setError(err.message || "Failed to load summary.");
    } finally {
      setLoadingSummary(false);
    }
  }

  // ------------------------
  // Entries
  // ------------------------

  async function loadEntriesForUser(username, filters = entryFilters) {
    setEntriesLoading(true);
    setError("");

    try {
      const data = await listEntries(username, {
        category: filters.category,
        search: filters.search,
        from: filters.from,
        to: filters.to,
        sort: filters.sort,
        order: filters.order,
        limit: filters.limit,
        offset: filters.offset,
      });
      setEntries(data.entries || data);
      setTotalCount(data.total_count ?? (data.entries?.length ?? 0));
    } catch (err) {
      console.error(err);
      setError(err.message || "Failed to load entries.");
    } finally {
      setEntriesLoading(false);
    }
  }

  async function handleCreateEntry(entryData) {
    if (!user) return;
    setError("");

    try {
      await createEntry(user.username, entryData);
      await loadEntriesForUser(user.username, entryFilters);
      await loadSummaryForUser(user.username);
    } catch (err) {
      console.error(err);
      throw err;
    }
  }

  async function handleUpdateEntry(entryId, patch) {
    if (!user) return;
    setError("");

    try {
      await updateEntry(user.username, entryId, patch);
      setEditingEntryId(null);
      await loadEntriesForUser(user.username, entryFilters);
      await loadSummaryForUser(user.username);
    } catch (err) {
      console.error(err);
      setError(err.message || "Failed to update entry.");
      throw err;
    }
  }

  async function handleDeleteEntry(entryId) {
    if (!user) return;

    const ok = window.confirm("Are you sure you want to delete this entry?");
    if (!ok) return;

    setError("");
    try {
      await deleteEntry(user.username, entryId);
      setEditingEntryId(null);
      await loadEntriesForUser(user.username, entryFilters);
      await loadSummaryForUser(user.username);
    } catch (err) {
      console.error(err);
      setError(err.message || "Failed to delete entry.");
    }
  }

  async function handleExport(format) {
    if (!user) return;
    setExporting(true);
    setError("");
    setExportSuccess(false);

    try {
      if (format === "json") {
        const data = await exportUserData(user.username, "json");
        const blob = new Blob([JSON.stringify(data, null, 2)], {
          type: "application/json",
        });
        const a = document.createElement("a");
        a.href = URL.createObjectURL(blob);
        a.download = `atlas_export_${user.username}.json`;
        a.click();
        URL.revokeObjectURL(a.href);
      } else {
        await exportUserData(user.username, "csv");
      }
      setExportSuccess(true);
      setTimeout(() => setExportSuccess(false), 2500);
    } catch (err) {
      console.error(err);
      setError(err.message || "Export failed.");
    } finally {
      setExporting(false);
    }
  }

  function handleFiltersChange(newFilters) {
    const merged = { ...entryFilters, ...newFilters };
    setEntryFilters(merged);
    if (user) {
      loadEntriesForUser(user.username, merged);
    }
  }

  async function goToDashboard() {
    if (!user) return;
    setView("dashboard");
    await loadSummaryForUser(user.username);
  }

  async function goToEntries() {
    if (!user) return;
    setView("entries");
    await loadEntriesForUser(user.username, entryFilters);
  }

  // ------------------------
  // View render helpers
  // ------------------------

  function renderDashboardView() {
    if (!user) return null;

    return (
      <div className="card">
        <h1>Dashboard</h1>
        <p className="card-subtitle">
          Overview of your academic activity and database-backed analytics.
        </p>

        <div className="dashboard-actions">
          <button
            className="secondary-button"
            type="button"
            onClick={() => loadSummaryForUser(user.username)}
            disabled={loadingSummary}
          >
            {loadingSummary ? "Refreshing..." : "Refresh"}
          </button>
          <div className="export-buttons">
            <span className="export-label">Export:</span>
            {exportSuccess && (
              <span className="export-success">Downloaded ✓</span>
            )}
            <button
              type="button"
              className="secondary-button export-btn"
              onClick={() => handleExport("json")}
              disabled={exporting}
              title="Download as JSON"
            >
              {exporting ? "..." : "JSON"}
            </button>
            <button
              type="button"
              className="secondary-button export-btn"
              onClick={() => handleExport("csv")}
              disabled={exporting}
              title="Download as CSV"
            >
              {exporting ? "..." : "CSV"}
            </button>
          </div>
        </div>

        {!summary && !loadingSummary && (
          <p className="hint">
            No summary yet. Try adding entries or importing legacy data, then
            refresh.
          </p>
        )}

        {loadingSummary && !summary && (
          <p className="hint loading-placeholder">Loading summary...</p>
        )}

        {summary && (
          <>
            <div className="summary-grid">
              <SummaryStat
                label="Total entries"
                value={summary.total_entries}
              />
              <SummaryStat
                label="First entry"
                value={summary.first_entry_date || "—"}
              />
              <SummaryStat
                label="Most recent"
                value={summary.last_entry_date || "—"}
              />
            </div>

            <h2 className="section-title">Entries per category</h2>
            <div className="category-list">
              {Object.entries(summary.entries_per_category || {}).map(
                ([category, count]) => (
                  <div key={category} className="category-item">
                    <span className="category-name">{category}</span>
                    <span className="category-count">{count}</span>
                  </div>
                )
              )}
            </div>

            {summary.timeline_by_month?.length > 0 && (
              <>
                <h2 className="section-title">Activity by month</h2>
                <div className="timeline-table-wrap">
                  <table className="timeline-table">
                    <thead>
                      <tr>
                        <th>Month</th>
                        <th>Entries</th>
                      </tr>
                    </thead>
                    <tbody>
                      {summary.timeline_by_month.map(({ period, count }) => (
                        <tr key={period}>
                          <td>{period}</td>
                          <td>{count}</td>
                        </tr>
                      ))}
                    </tbody>
                  </table>
                </div>
              </>
            )}
          </>
        )}
      </div>
    );
  }

  function renderEntriesView() {
    if (!user) return null;

    const editingEntry = editingEntryId
      ? entries.find((e) => e.id === editingEntryId)
      : null;

    return (
      <div className="card">
        <h1>Entries</h1>
        <p className="card-subtitle">
          View, filter, add, edit, and remove entries in your database.
        </p>

        <EntriesFilters
          filters={entryFilters}
          categories={categories}
          onFiltersChange={handleFiltersChange}
          onRefresh={() => loadEntriesForUser(user.username, entryFilters)}
          loading={entriesLoading}
        />

        <h2 className="section-title">Add new entry</h2>
        <EntryForm
          categories={categories}
          onSubmit={handleCreateEntry}
          loading={entriesLoading}
        />

        {editingEntry && (
          <>
            <h2 className="section-title">Edit entry</h2>
            <EntryForm
              key={editingEntryId}
              categories={categories}
              onSubmit={async (data) => {
                await handleUpdateEntry(editingEntryId, data);
              }}
              onCancel={() => setEditingEntryId(null)}
              loading={entriesLoading}
              submitLabel="Save changes"
              initialCategory={editingEntry.category}
              initialDate={editingEntry.date}
              initialDetails={
                editingEntry.category === "AP Scores" ? "" : editingEntry.details
              }
              initialApExam={
                editingEntry.category === "AP Scores" &&
                editingEntry.details?.includes(" - ")
                  ? editingEntry.details.split(" - ")[0]?.trim() ?? ""
                  : ""
              }
              initialApScore={
                editingEntry.category === "AP Scores" &&
                editingEntry.details?.includes(" - ")
                  ? editingEntry.details.split(" - ")[1]?.trim() ?? ""
                  : ""
              }
            />
          </>
        )}

        <h2 className="section-title" style={{ marginTop: "1.5rem" }}>
          Existing entries
          {totalCount > 0 && (
            <span className="entries-count"> ({totalCount})</span>
          )}
        </h2>
        <EntriesTable
          entries={entries}
          loading={entriesLoading}
          onEditEntry={(id) => setEditingEntryId(id)}
          onDeleteEntry={handleDeleteEntry}
          showActions={true}
        />
        <EntriesPagination
          filters={entryFilters}
          totalCount={totalCount}
          onFiltersChange={handleFiltersChange}
          loading={entriesLoading}
        />
      </div>
    );
  }

  // ------------------------
  // Main render
  // ------------------------

  return (
    <div className="app-root">
      <Navbar
        username={user?.username || null}
        currentView={view}
        onDashboardClick={goToDashboard}
        onEntriesClick={goToEntries}
        onExport={user ? (format) => handleExport(format) : null}
        onLogout={handleLogout}
      />

      <main className="app-main">
        {user && error && <div className="error-banner">{error}</div>}

        {!user && view === "login" && (
          <LoginPage
            onLogin={handleLoginExisting}
            onCreateAccount={handleCreateAccount}
            loading={loadingAuth}
            error={error}
            infoMessage={authMessage}
            apiBaseUrl={API_BASE_URL}
          />
        )}

        {user && view === "dashboard" && renderDashboardView()}
        {user && view === "entries" && renderEntriesView()}
      </main>
    </div>
  );
}

function SummaryStat({ label, value }) {
  return (
    <div className="summary-stat">
      <div className="summary-label">{label}</div>
      <div className="summary-value">{value}</div>
    </div>
  );
}

export default App;
