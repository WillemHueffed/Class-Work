import { it, describe, expect, beforeAll } from "@jest/globals";
import request from "supertest";
import app from "../app";
import { Book, books, Author, authors, Edition } from "../data";

describe("Book Router", () => {
  beforeAll(async () => {
    for (let i = 1; i < 11; i++) {
      const author: Author = new Author(
        `name${i}`,
        `bio${i}`,
        `bday${i}`,
        `genre${i}`,
      );
      const book: Book = new Book(
        `book${i}`,
        `sub${i}`,
        `pub${i}`,
        [`tag${i}`],
        author,
      );
      authors.push(author);
      books.push(book);
    }
    for (let i = 1; i < books.length / 2; i++) {
      for (let j = 1; j < 4; j++) {
        const edition: Edition = new Edition(j, `date${j}`);
        books[i].editions.push(edition);
      }
    }
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
      const id = authors[0].id;
      const response = await request(app).get(`/books/byAuthor/${id}`);

      expect(response.statusCode).toBe(200);
      expect(Array.isArray(response.body)).toBe(true);

      const filteredBooks = books.filter((book) => book.p_auth.id === id);

      expect(response.body).toEqual(filteredBooks);
    });
  });
});

describe("GET /books/:id", () => {
  it("returns book filtered by id", async () => {
    const id = books[0].id;
    const response = await request(app).get(`/books/${id}`);

    expect(response.statusCode).toBe(200);
    expect(response.body).toEqual(books[0]);
  });
});

describe("PATCH /books/:id", () => {
  it("updates book and returns updated object", async () => {
    const id = books[0].id;
    const fields = {
      title: "patched title",
      subtitle: "patched sub title",
      org_pub_date: "patched pub date",
      tags: ["patched tag"],
    };

    const response = await request(app).patch(`/books/${id}`).send(fields);
    expect(response.statusCode).toBe(200);
    expect(response.body).toMatchObject(fields);
  });

  describe("DELETE /books/:id", () => {
    it("deletes target book", async () => {
      const id = books[0].id;
      const response = await request(app).delete(`/books/${id}`);
      expect(response.statusCode).toBe(204);
      expect(books[0].id).not.toBe(id);
    });
  });
});
