import { it, describe, expect } from "@jest/globals";
import request from "supertest";
import app from "../app";

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
});
