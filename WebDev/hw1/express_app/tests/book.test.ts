import { it, describe, expect } from "@jest/globals";
import request from "supertest";
import app from "../app";
import { Book, books, Author, authors } from "../data";

describe("Book Router", () => {
  describe("GET /books", () => {
    it("should return all books", async () => {
      const response = await request(app).get("/books");
      expect(response.status).toBe(200);
      expect(Array.isArray(response.body)).toBe(true);
    });
  });

  describe("POST /books", () => {
    it("should add a new book", async () => {
      const newBook = {
        title: "New Book Title",
        subtitle: "New Book Subtitle",
        org_pub_date: "2022-01-01",
        tags: ["Tag1", "Tag2"],
        p_auth: {
          name: "Author Name",
          bio: "Author Bio",
          bday: "2000-01-01",
          genre: "Fiction",
        },
      };
      const response = await request(app)
        .post("/books")
        .send(newBook)
        .set("Accept", "application/json");
      expect(response.status).toBe(200);
      expect(response.body.book).toHaveProperty("id");
    });
  });

  describe("GET /books/byTags", () => {
    it("returns books filtered by tags", async () => {
      const tags = ["Tag1", "Tag2"]; // Specify the tags to filter by
      const response = await request(app).get("/books/byTags").query({ tags }); // Send the tags as query parameters
      expect(response.statusCode).toBe(200);
      expect(response.body).toEqual(expect.any(Array));
      response.body.forEach((book: Book) => {
        expect(book.tags).toEqual(expect.arrayContaining(tags));
      });
    });
  });

  describe("GET /books/byAuthor/:authorID", () => {
    it("returns books filtered by author", async () => {
      const authorID = "sample_author_id";
      const response = await request(app).get(`/books/byAuthor/${authorID}`);

      expect(response.statusCode).toBe(200);
      expect(Array.isArray(response.body)).toBe(true);

      response.body.forEach((book: Book) => {
        expect(book.id).toBe(authorID);
      });
    });
  });

  describe("GET /books/:id", () => {
    const new_author = new Author("will", "bio", "bday", "scifi");
    const new_book = new Book(
      "book",
      "subtitle",
      "4/5/24",
      ["t1", "t2"],
      new_author,
    );
    authors.push(new_author);
    books.push(new_book);

    it("returns book details filtered by book's id", async () => {
      const found_book = books.find((book) => book.title === new_book.title);
      let bookID = null;
      if (found_book) bookID = found_book.id;
      else return;

      const response = await request(app).get(`/books/${bookID}`);

      expect(response.statusCode).toBe(200);
      expect(response.body).toEqual({ new_book });
    });
  });
});
