curl -X POST \
  -v \
  http://localhost:3002/reviews/60db65e7-fc2a-492f-9e20-af16f2754878/comments \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1c2VybmFtZSI6IndoZXVmZmVkQHNlYXR0bGV1LmVkdSIsInBhc3N3b3JkIjoicGFzc3dvcmQxMjMiLCJleHAiOjE3MTY1NTYwMDR9.i-2Hr7sDgG58Gx7jlCF8qBEdajYQpS16W-Ik_dPOIBs" \
  -d '{ 
      "comment" : "a comment on a review"
      }'
