curl -X POST \
  -v \
  http://localhost:3002/reviews/41dbb88d-5812-4a5d-aa2b-270990542e1a/comments \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1c2VybmFtZSI6IndoZXVmZmVkQHNlYXR0bGV1LmVkdSIsInBhc3N3b3JkIjoicGFzc3dvcmQxMjMiLCJleHAiOjE3MTY1NTYwMDR9.i-2Hr7sDgG58Gx7jlCF8qBEdajYQpS16W-Ik_dPOIBs" \
  -d '{ 
      "comment" : "a comment on a review"
      }'
