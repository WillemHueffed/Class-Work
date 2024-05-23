curl -X POST \
  -v \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1c2VybmFtZSI6IndoZXVmZmVkQHNlYXR0bGV1LmVkdSIsImV4cCI6MTcxNjU1NTc0N30.EsAi_QXkYzDnSPQbaHjaEhQzQS7QGY3TQCTF34j0RG8" \
  -d '{"desc": "desc", "rating": "5"}' \
  http://localhost:3002/login

