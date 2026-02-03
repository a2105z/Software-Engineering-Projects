// frontend/src/api.js

// In dev, use /api prefix so Vite proxy forwards to backend (avoids CORS).
// In production, use env var or explicit backend URL.
const API_BASE_URL = import.meta.env.DEV
  ? "/api"
  : (import.meta.env.VITE_API_URL || "http://localhost:8001");

async function fetchJson(url, options = {}) {
  const resp = await fetch(url, {
    headers: {
      "Content-Type": "application/json",
    },
    ...options,
  });

  const text = await resp.text();
  let data = null;

  try {
    data = text ? JSON.parse(text) : null;
  } catch (e) {
    // ignore parse errors
  }

  if (!resp.ok) {
    const message =
      (data && data.detail) ||
      `Request failed with status ${resp.status}`;
    const err = new Error(message);
    err.status = resp.status;
    err.data = data;
    throw err;
  }

  return data;
}

// -------- Auth --------

export async function registerUser({
  username,
  first_name,
  last_name,
  birthday,
  password,
}) {
  return fetchJson(`${API_BASE_URL}/users/`, {
    method: "POST",
    body: JSON.stringify({
      username,
      first_name,
      last_name,
      birthday,
      password,
    }),
  });
}

export async function loginUser(username, password) {
  return fetchJson(`${API_BASE_URL}/auth/login`, {
    method: "POST",
    body: JSON.stringify({ username, password }),
  });
}

export async function getUser(username) {
  return fetchJson(`${API_BASE_URL}/users/${encodeURIComponent(username)}`);
}

// -------- Categories & entries --------

export async function getCategories(username) {
  return fetchJson(
    `${API_BASE_URL}/users/${encodeURIComponent(username)}/categories`
  );
}

export async function listEntries(username, params = {}) {
  const qs = new URLSearchParams();
  if (params.category) qs.set("category", params.category);
  if (params.search) qs.set("search", params.search);
  if (params.from) qs.set("from", params.from);
  if (params.to) qs.set("to", params.to);
  if (params.sort) qs.set("sort", params.sort);
  if (params.order) qs.set("order", params.order);
  if (params.limit != null) qs.set("limit", params.limit);
  if (params.offset != null) qs.set("offset", params.offset);

  const queryString = qs.toString() ? `?${qs.toString()}` : "";

  const data = await fetchJson(
    `${API_BASE_URL}/users/${encodeURIComponent(
      username
    )}/entries${queryString}`
  );
  return data;
}

export async function getEntry(username, entryId) {
  return fetchJson(
    `${API_BASE_URL}/users/${encodeURIComponent(
      username
    )}/entries/${entryId}`
  );
}

export async function createEntry(username, data) {
  return fetchJson(
    `${API_BASE_URL}/users/${encodeURIComponent(username)}/entries`,
    {
      method: "POST",
      body: JSON.stringify(data),
    }
  );
}

export async function updateEntry(username, entryId, data) {
  return fetchJson(
    `${API_BASE_URL}/users/${encodeURIComponent(
      username
    )}/entries/${entryId}`,
    {
      method: "PATCH",
      body: JSON.stringify(data),
    }
  );
}

export async function deleteEntry(username, entryId) {
  await fetchJson(
    `${API_BASE_URL}/users/${encodeURIComponent(
      username
    )}/entries/${entryId}`,
    { method: "DELETE" }
  );
  return true;
}

// -------- Analytics --------

export async function getAnalyticsSummary(username) {
  return fetchJson(
    `${API_BASE_URL}/analytics/${encodeURIComponent(username)}/summary`
  );
}

// -------- Reports --------

export async function getTimelineReport(username, groupBy = "month") {
  return fetchJson(
    `${API_BASE_URL}/reports/${encodeURIComponent(
      username
    )}/timeline?group_by=${encodeURIComponent(groupBy)}`
  );
}

export async function getByCategoryReport(username) {
  return fetchJson(
    `${API_BASE_URL}/reports/${encodeURIComponent(username)}/by-category`
  );
}

export async function getActivitySummaryReport(username) {
  return fetchJson(
    `${API_BASE_URL}/reports/${encodeURIComponent(
      username
    )}/activity-summary`
  );
}

// -------- Export --------

export async function exportUserData(username, format = "json") {
  const url = `${API_BASE_URL}/users/${encodeURIComponent(
    username
  )}/export?format=${format}`;
  const resp = await fetch(url);
  if (!resp.ok) {
    const text = await resp.text();
    let err;
    try {
      err = JSON.parse(text);
    } catch {
      err = { detail: text };
    }
    throw new Error(err.detail || `Export failed: ${resp.status}`);
  }
  if (format === "json") {
    return resp.json();
  }
  const blob = await resp.blob();
  const a = document.createElement("a");
  a.href = URL.createObjectURL(blob);
  a.download = `atlas_export_${username}.csv`;
  a.click();
  URL.revokeObjectURL(a.href);
  return true;
}

export { API_BASE_URL };
