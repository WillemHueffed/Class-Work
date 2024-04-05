import { it, describe, expect } from "@jest/globals";
import request from "supertest";
import app from "../app";
import { authors } from "../data";

describe("GET /authors", () => {
  it("responds with JSON containing all authors", async () => {
    const response = await request(app).get("/authors");
    expect(response.statusCode).toBe(200);
    expect(response.body.authors).toEqual(authors);
  });
});

describe("POST /authors", () => {
  it("adds a new author and responds with the added author", async () => {
    const newAuthor = {
      name: "John Doe",
      bio: "Author bio",
      bday: "2000-01-01",
      genre: "Fiction",
    };
    const response = await request(app)
      .post("/authors")
      .send(newAuthor)
      .set("Accept", "application/json");
    expect(response.statusCode).toBe(200);
    expect(response.body.author).toEqual(expect.objectContaining(newAuthor));
  });
});

describe("PATCH /authors/:authorID", () => {
  it("updates the bio of the specified author and responds with the updated author", async () => {
    const authorID = authors[0].id; // Assuming the first author in the data array
    const updatedBio = "Updated author bio";
    const response = await request(app)
      .patch(`/authors/${authorID}`)
      .send({ bio: updatedBio })
      .set("Accept", "application/json");
    expect(response.statusCode).toBe(200);
    expect(response.body.author.bio).toEqual(updatedBio);
  });
});
