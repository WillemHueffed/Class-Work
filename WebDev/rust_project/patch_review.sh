curl -X PATCH \
  -v \
  http://localhost:3002/reviews/da3d7c44-50e5-405c-a9ba-5d903d2b50b9 \
  -H 'Content-Type: application/json' \
  -d '{"desc": "patched desc", "rating": "13"}'

