curl -X POST \
  -v \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1c2VybmFtZSI6IndoZXVmZmVkQHNlYXR0bGV1LmVkdSIsInBhc3N3b3JkIjoicGFzc3dvcmQxMjMiLCJleHAiOjE3MTY1NjAzMzF9.Nss7Fpit_7uY7BrkaKVU51xSbOWeajd6hSavVzpdcHQ" \
  -d '{"desc": "desc", "rating": "5"}' \
  http://localhost:3002/login

