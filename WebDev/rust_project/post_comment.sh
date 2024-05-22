curl -X POST \
  -v \
  http://localhost:3002/reviews/60db65e7-fc2a-492f-9e20-af16f2754878/comments \
  -H 'Content-Type: application/json' \
  -d '{"comment" : "a comment on a review"}'
