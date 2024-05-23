curl -X POST \
  -v \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJlbWFpbCI6IndoZXVmZmVkQHNlYXR0bGV1LmVkdSIsImV4cCI6MTcxNjQ4MDY3Nn0.AWNUlroZSafeaItCdJlMqmXM90nYDCnQCTXLjWVYJ_E" \
  -d '{"username": "test_username", "password": "password123"}' \
  http://localhost:3002/login

