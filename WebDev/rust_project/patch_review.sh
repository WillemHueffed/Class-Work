curl -X PATCH \
  -v \
  http://localhost:3002/reviews/be6bf75e-e8fa-46ad-8909-cf0f5bc5f9c9 \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1c2VybmFtZSI6InRlc3RfdXNlcm5hbWUiLCJwYXNzd29yZCI6InBhc3N3b3JkMTIzIiwiZXhwIjoxNzE2NTYwNTk2fQ.oixG-FAIKNG7nzDgCx5nijP-92uaTjL4ZEqMQKsiKg4" \
  -d '{"desc": "patched desc", "rating": "13"}'

