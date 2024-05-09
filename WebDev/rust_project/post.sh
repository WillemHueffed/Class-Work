curl -X POST \
  -v \
  -H 'Content-Type: application/json' \
  http://localhost:3002/reviews/1 \
  -d '{"description": "desc", "rating": "5"}'

