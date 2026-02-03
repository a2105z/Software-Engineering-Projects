import React from "react";

function Navbar({
  appTitle = "Atlas",
  username = null,
  currentView = "",
  onDashboardClick,
  onEntriesClick,
  onExport,
  onLogout,
}) {
  const isLoggedIn = Boolean(username);

  return (
    <header className="navbar">
      <div className="navbar-left">
        <span className="app-title">{appTitle}</span>
      </div>

      <div className="navbar-right">
        {isLoggedIn && (
          <>
            <button
              type="button"
              className={
                "nav-button" +
                (currentView === "dashboard" ? " nav-button-active" : "")
              }
              onClick={onDashboardClick}
            >
              Dashboard
            </button>

            <button
              type="button"
              className={
                "nav-button" +
                (currentView === "entries" ? " nav-button-active" : "")
              }
              onClick={onEntriesClick}
            >
              Entries
            </button>

            <span className="nav-username">
              Signed in as <strong>{username}</strong>
            </span>

            {onExport && (
              <button
                type="button"
                className="nav-button nav-export"
                onClick={() => onExport("csv")}
                title="Export my data (CSV)"
              >
                Export
              </button>
            )}

            <button
              type="button"
              className="nav-button nav-logout"
              onClick={onLogout}
            >
              Logout
            </button>
          </>
        )}
      </div>
    </header>
  );
}

export default Navbar;
