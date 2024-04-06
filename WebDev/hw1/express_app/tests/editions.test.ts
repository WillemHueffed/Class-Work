import { it, describe, expect, beforeAll } from "@jest/globals";
import request from "supertest";
import app from "../app";
import { Book, books, Author, authors, Edition } from "../data";

describe("Edition Router", () => {
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

  describe("POST /Editions", () => {
    it("should add a new book", async () => {
      const id = books[0].id;
      const fields = {
        ed_num: 420,
        pub_date: "6969",
      };
      const response = await request(app)
        .post(`/books/${id}/editions`)
        .send(fields);

      delete response.body.id;
      expect(response.status).toBe(200);
      expect(response.body).toEqual(fields);
    });
  });

  describe("GET /editions", () => {
    it("should return all editions of a book", async () => {
      const book_id = books[0].id;
      const response = await request(app).get(`/books/${book_id}/editions`);

      expect(response.status).toBe(200);
      expect(response.body).toEqual(books[0].editions);
    });
  });

  describe("DELETE /editions/:editionID", () => {
    it("should delete target book", async () => {
      const book_id = books[0].id;
      const ed_id = books[0].editions[0].id;
      const response = await request(app).delete(
        `/books/${book_id}/editions/${ed_id}`,
      );
      console.log(response.headers);
      console.log(response.body);
      expect(response.status).toBe(204);
      expect(books[0].editions[0].id).not.toBe(ed_id);
    });
  });
});
