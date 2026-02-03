import React from "react";

function CategoryList({
  categories = [],
  selectedCategory = null,
  onSelectCategory,
  showAllOption = true,
}) {
  const handleClick = (category) => {
    if (!onSelectCategory) return;
    onSelectCategory(category);
  };

  const isSelected = (category) =>
    (category === null && selectedCategory === null) ||
    (category !== null && category === selectedCategory);

  return (
    <div className="category-list">
      {showAllOption && (
        <button
          type="button"
          className={
            "category-item" +
            (isSelected(null) ? " category-item-selected" : "")
          }
          style={{
            textAlign: "left",
            borderStyle: "dashed",
          }}
          onClick={() => handleClick(null)}
        >
          <span className="category-name">All categories</span>
          {/* You could show total count here if you pass it via props later */}
        </button>
      )}

      {categories.map((category) => (
        <button
          key={category}
          type="button"
          className={
            "category-item" +
            (isSelected(category) ? " category-item-selected" : "")
          }
          style={{ textAlign: "left" }}
          onClick={() => handleClick(category)}
        >
          <span className="category-name">{category}</span>
        </button>
      ))}
    </div>
  );
}

export default CategoryList;
