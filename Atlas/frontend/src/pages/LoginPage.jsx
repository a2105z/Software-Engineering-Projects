// frontend/src/pages/LoginPage.jsx

import React, { useState } from "react";

/**
 * LoginPage
 *
 * Two sections:
 *  1) Log in with username + password
 *  2) Create account with first/last/birthday/username/password
 *
 * Props:
 *  - onLogin: ({ username, password }) => void | Promise
 *  - onCreateAccount: ({ firstName, lastName, birthday, username, password }) => void | Promise
 *  - loading: boolean
 *  - error: string           (shown near login)
 *  - infoMessage: string     (e.g. "Account created, now log in")
 *  - apiBaseUrl: string
 */
function LoginPage({
  onLogin,
  onCreateAccount,
  loading = false,
  error = "",
  infoMessage = "",
  apiBaseUrl = "http://localhost:8001",
}) {
  // Login form state
  const [loginUsername, setLoginUsername] = useState("");
  const [loginPassword, setLoginPassword] = useState("");

  // Signup form state
  const [firstName, setFirstName] = useState("");
  const [lastName, setLastName] = useState("");
  const [birthday, setBirthday] = useState("");
  const [signupUsername, setSignupUsername] = useState("");
  const [signupPassword, setSignupPassword] = useState("");

  const handleLoginSubmit = (e) => {
    e.preventDefault();
    onLogin?.({
      username: loginUsername.trim(),
      password: loginPassword,
    });
  };

  const handleSignupSubmit = (e) => {
    e.preventDefault();
    onCreateAccount?.({
      firstName: firstName.trim(),
      lastName: lastName.trim(),
      birthday, // "YYYY-MM-DD"
      username: signupUsername.trim(),
      password: signupPassword,
    });
  };

  return (
    <div className="card">
      <h1>Welcome to Atlas</h1>
      <p className="card-subtitle">
        Track your academic journey with a database-backed Python backend.
      </p>

      {/* Login section */}
      {error && <div className="error-banner">{error}</div>}

      <form onSubmit={handleLoginSubmit} className="form">
        <h2 className="section-title">Log in</h2>

        <label className="form-label">
          Username
          <input
            type="text"
            value={loginUsername}
            onChange={(e) => setLoginUsername(e.target.value)}
            className="form-input"
            placeholder="Enter your username"
          />
        </label>

        <label className="form-label">
          Password
          <input
            type="password"
            value={loginPassword}
            onChange={(e) => setLoginPassword(e.target.value)}
            className="form-input"
            placeholder="Enter your password"
          />
        </label>

        <button
          type="submit"
          className="primary-button"
          disabled={loading}
        >
          {loading ? "Working..." : "Log in"}
        </button>
      </form>

      {/* Divider */}
      <hr
        style={{
          margin: "1.5rem 0",
          borderColor: "rgba(148,163,184,0.4)",
        }}
      />

      {/* Create account section */}
      <form onSubmit={handleSignupSubmit} className="form">
        <h2 className="section-title">Create account</h2>

        {infoMessage && (
          <p className="hint" style={{ marginBottom: "0.5rem" }}>
            {infoMessage}
          </p>
        )}

        <label className="form-label">
          First name
          <input
            type="text"
            value={firstName}
            onChange={(e) => setFirstName(e.target.value)}
            className="form-input"
            placeholder="First name"
          />
        </label>

        <label className="form-label">
          Last name
          <input
            type="text"
            value={lastName}
            onChange={(e) => setLastName(e.target.value)}
            className="form-input"
            placeholder="Last name"
          />
        </label>

        <label className="form-label">
          Birthday
          <input
            type="date"
            value={birthday}
            onChange={(e) => setBirthday(e.target.value)}
            className="form-input"
          />
        </label>

        <label className="form-label">
          Username
          <input
            type="text"
            value={signupUsername}
            onChange={(e) => setSignupUsername(e.target.value)}
            className="form-input"
            placeholder="Choose a username"
          />
        </label>

        <label className="form-label">
          Password
          <input
            type="password"
            value={signupPassword}
            onChange={(e) => setSignupPassword(e.target.value)}
            className="form-input"
            placeholder="Choose a password"
          />
        </label>

        <button
          type="submit"
          className="secondary-button"
          disabled={loading}
        >
          {loading ? "Working..." : "Create account"}
        </button>
      </form>

      <p className="hint" style={{ marginTop: "1rem" }}>
        This talks to your FastAPI backend
        {apiBaseUrl && apiBaseUrl.startsWith("/") ? (
          <> (proxied to <code>localhost:8001</code>)</>
        ) : apiBaseUrl ? (
          <> at <code>{apiBaseUrl}</code></>
        ) : null}
        . Accounts are stored locally (username + hashed password).
      </p>
    </div>
  );
}

export default LoginPage;
