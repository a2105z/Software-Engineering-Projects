import React from "react";

function EntriesPage({
  entries = [],
  loading = false,
  onRefresh,
  onDeleteEntry,
  filters = {},
  onFiltersChange,
  categories = [],
}) {
  const handleCategoryChange = (e) => {
    if (!onFiltersChange) return;
    onFiltersChange({
      ...filters,
      category: e.target.value || undefined,
    });
  };

  const handleSearchChange = (e) => {
    if (!onFiltersChange) return;
    onFiltersChange({
      ...filters,
      search: e.target.value || undefined,
    });
  };

  const hasFilters = Boolean(filters.category || filters.search);

  return (
    <div className="card">
      <h1>Entries</h1>
      <p className="card-subtitle">
        These are the entries stored in your backend database. You can refresh
        the list or filter by category and search term.
      </p>

      {/* Controls: refresh + filters */}
      <div
        style={{
          display: "flex",
          flexWrap: "wrap",
          gap: "0.75rem",
          alignItems: "center",
          marginBottom: "0.75rem",
        }}
      >
        <button
          className="secondary-button"
          type="button"
          onClick={onRefresh}
          disabled={loading}
        >
          {loading ? "Refreshing..." : "Refresh entries"}
        </button>

        {onFiltersChange && (
          <>
            <div style={{ display: "flex", flexDirection: "column", gap: "0.25rem" }}>
              <label
                style={{
                  fontSize: "0.8rem",
                  color: "#e5e7eb",
                }}
              >
                Category
              </label>
              <select
                value={filters.category || ""}
                onChange={handleCategoryChange}
                className="form-input"
                style={{ minWidth: "180px", paddingRight: "2rem" }}
              >
                <option value="">All categories</option>
                {categories.map((cat) => (
                  <option key={cat} value={cat}>
                    {cat}
                  </option>
                ))}
              </select>
            </div>

            <div style={{ display: "flex", flexDirection: "column", gap: "0.25rem" }}>
              <label
                style={{
                  fontSize: "0.8rem",
                  color: "#e5e7eb",
                }}
              >
                Search
              </label>
              <input
                type="text"
                value={filters.search || ""}
                onChange={handleSearchChange}
                placeholder="Search in details..."
                className="form-input"
                style={{ minWidth: "220px" }}
              />
            </div>
          </>
        )}
      </div>

      {/* Loading / empty states */}
      {loading && entries.length === 0 && <p>Loading entries...</p>}

      {!loading && entries.length === 0 && (
        <p className="hint">
          No entries match your current filters{hasFilters ? "" : ""}. Try
          adding entries via the backend scripts or API, then refresh.
        </p>
      )}

      {/* Entries table */}
      {entries.length > 0 && (
        <table className="entries-table">
          <thead>
            <tr>
              <th style={{ width: "120px" }}>Date</th>
              <th style={{ width: "190px" }}>Category</th>
              <th>Details</th>
              {onDeleteEntry && <th style={{ width: "80px" }}>Actions</th>}
            </tr>
          </thead>
          <tbody>
            {entries.map((entry) => (
              <tr key={entry.id}>
                <td>{entry.date}</td>
                <td>{entry.category}</td>
                <td>{entry.details}</td>
                {onDeleteEntry && (
                  <td>
                    <button
                      type="button"
                      className="nav-button nav-logout"
                      style={{ fontSize: "0.75rem", padding: "0.25rem 0.5rem" }}
                      onClick={() => onDeleteEntry(entry.id)}
                    >
                      Delete
                    </button>
                  </td>
                )}
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </div>
  );
}

export default EntriesPage;
