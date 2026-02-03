import React from "react";

function DashboardPage({
  summary,
  prediction,
  loadingSummary = false,
  loadingPrediction = false,
  onRefreshSummary,
  onRunPrediction,
}) {
  return (
    <div className="card">
      <h1>Dashboard</h1>
      <p className="card-subtitle">
        Overview of your activity and a simple ML-style prediction.
      </p>

      {/* Summary refresh button */}
      <div style={{ marginBottom: "0.75rem" }}>
        <button
          className="secondary-button"
          type="button"
          onClick={onRefreshSummary}
          disabled={loadingSummary}
        >
          {loadingSummary ? "Refreshing..." : "Refresh summary"}
        </button>
      </div>

      {/* Summary content */}
      {!summary && !loadingSummary && (
        <p className="hint">
          No summary data yet. Try adding entries via the backend or scripts,
          then refresh.
        </p>
      )}

      {loadingSummary && !summary && <p>Loading summary...</p>}

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
              label="Most recent entry"
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
        </>
      )}

      {/* Prediction section */}
      <h2 className="section-title">Prediction</h2>

      {prediction ? (
        <PredictionCard prediction={prediction} />
      ) : (
        <button
          className="primary-button"
          type="button"
          onClick={onRunPrediction}
          disabled={loadingPrediction}
        >
          {loadingPrediction ? "Running prediction..." : "Run prediction"}
        </button>
      )}
    </div>
  );
}

/**
 * Small stat card used in the summary grid.
 */
function SummaryStat({ label, value }) {
  return (
    <div className="summary-stat">
      <div className="summary-label">{label}</div>
      <div className="summary-value">{value}</div>
    </div>
  );
}

/**
 * PredictionCard used to display the ML/heuristic result.
 */
function PredictionCard({ prediction }) {
  const probabilityPct = (prediction.probability * 100).toFixed(1);

  return (
    <div className="prediction-card">
      <div className="prediction-row">
        <span className="prediction-label">Estimated probability:</span>
        <span className="prediction-value">{probabilityPct}%</span>
      </div>

      <div className="prediction-row">
        <span className="prediction-label">Model used:</span>
        <span className="prediction-badge">
          {prediction.model_used ? "Trained model" : "Heuristic"}
        </span>
      </div>

      <p className="prediction-message">{prediction.message}</p>
    </div>
  );
}

export default DashboardPage;
