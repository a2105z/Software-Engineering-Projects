"""
Pydantic schemas for Atlas API.

Request/response models for validation and serialization.
"""

from datetime import date, datetime
from typing import Dict, List, Optional
from pydantic import BaseModel, Field


# User schemas
class UserBase(BaseModel):
    username: str = Field(..., min_length=1)


class UserCreate(BaseModel):
    username: str = Field(..., min_length=1)
    first_name: str = Field(..., min_length=1)
    last_name: str = Field(..., min_length=1)
    birthday: date
    password: str = Field(..., min_length=4)


class UserLogin(BaseModel):
    username: str = Field(..., min_length=1)
    password: str = Field(..., min_length=1)


class UserOut(BaseModel):
    id: int
    username: str
    first_name: str
    last_name: str
    birthday: date
    created_at: datetime

    class Config:
        from_attributes = True

# Entry schemas
class EntryBase(BaseModel):
    category: str
    date: date
    details: str


class EntryCreate(EntryBase):
    pass


class EntryUpdate(BaseModel):
    """Partial update; all fields optional."""
    category: Optional[str] = None
    date: Optional[date] = None
    details: Optional[str] = None


class EntryOut(BaseModel):
    id: int
    user_id: int
    category: str
    date: date
    details: str
    created_at: datetime

    class Config:
        from_attributes = True


# Entry list with pagination
class EntryListResponse(BaseModel):
    entries: List["EntryOut"]
    total_count: int


# Analytics schemas
class AnalyticsSummary(BaseModel):
    username: str
    total_entries: int
    entries_per_category: Dict[str, int]
    first_entry_date: Optional[date]
    last_entry_date: Optional[date]


# Report schemas
class TimelineGroup(BaseModel):
    period: str  # e.g. "2024-01", "2024-Q1"
    count: int


class TimelineReport(BaseModel):
    username: str
    group_by: str  # "month" or "quarter"
    groups: List[TimelineGroup]


class ByCategoryReport(BaseModel):
    username: str
    by_category: Dict[str, int]


class ActivitySummaryReport(BaseModel):
    username: str
    total_entries: int
    first_entry_date: Optional[date]
    last_entry_date: Optional[date]
    entries_per_category: Dict[str, int]
    timeline_by_month: List[TimelineGroup]
