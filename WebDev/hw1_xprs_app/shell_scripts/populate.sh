#!/bin/bash
# Function to create an author
create_author() {
  curl -X POST -s -H "Content-Type: application/json" -d '{
    "name": "Author'$1'",
    "bio": "Author bio",
    "bday": "2000-01-01",
    "genre": "Fiction"
  }' http://localhost:3000/authors
}

# Function to create a book
create_book() {
  curl -X POST -s -H "Content-Type: application/json" -d '{
    "title": "Book'$1'",
    "subtitle": "Subtitle'$1'",
    "org_pub_date": "2022-01-01",
    "tags": ["Tag'$1'"],
    "p_auth": {
      "name": "Author'$1'",
      "bio": "Author bio",
      "bday": "2000-01-01",
      "genre": "Fiction"
    }
  }' http://localhost:3000/books
}

# Create 3 authors
for i in {1..3}; do
  create_author $i
done

# Create 3 books
for i in {1..3}; do
  create_book $i
done

