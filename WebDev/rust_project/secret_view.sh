curl -X POST \
  -v \
  -H "Content-Type: application/json" \
  -d '{"email": "wheuffed@seattleu.edu", 
      "token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJlbWFpbCI6IndoZXVmZmVkQHNlYXR0bGV1LmVkdSIsImV4cCI6MTcxNjQ4MDY3Nn0.AWNUlroZSafeaItCdJlMqmXM90nYDCnQCTXLjWVYJ_E"}' \
  localhost:3002/get-token

