curl -X POST \
  -v \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1c2VybmFtZSI6InRlc3RfdXNlcm5hbWUiLCJwYXNzd29yZCI6InBhc3N3b3JkMTIzIiwiZXhwIjoxNzE2NTYwNTk2fQ.oixG-FAIKNG7nzDgCx5nijP-92uaTjL4ZEqMQKsiKg4" \
  -d '{"desc": "desc", "rating": "5"}' \
  http://localhost:3002/reviews/1

