curl -X POST \
  -v \
  http://localhost:3002/reviews/60db65e7-fc2a-492f-9e20-af16f2754878/comments \
  -H 'Content-Type: application/json' \
  -d '{ 
      "token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJlbWFpbCI6IndoZXVmZmVkQHNlYXR0bGV1LmVkdSIsImV4cCI6MTcxNjQ4MDY3Nn0.AWNUlroZSafeaItCdJlMqmXM90nYDCnQCTXLjWVYJ_E"}, 
      "comment" : "a comment on a review"
      }'
