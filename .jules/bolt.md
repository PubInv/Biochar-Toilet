
## 2024-05-18 - [Optimize High-Frequency Data Storage]
**Learning:** For high-frequency sensor data recording (e.g., storing a rolling window of history in `collections.deque`), allocating a new dictionary for each data point creates significant object creation overhead and uses ~60% more memory compared to using tuples.
**Action:** When storing simple, structured data points continuously in a rolling buffer, prefer tuples or namedtuples over dictionaries to reduce memory pressure and object creation time.
