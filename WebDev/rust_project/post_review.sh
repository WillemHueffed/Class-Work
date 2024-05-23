curl -X POST \
  -v \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJlbWFpbCI6IndoZXVmZmVkQHNlYXR0bGV1LmVkdSIsImV4cCI6MTcxNjQ4MDY3Nn0.AWNUlroZSafeaItCdJlMqmXM90nYDCnQCTXLjWVYJ_E" \
  -d '{"desc": "desc", "rating": "5"}' \
  http://localhost:3002/reviews/1

