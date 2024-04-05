import { it, describe, expect, test } from "@jest/globals";
import request from "supertest";
import app from "../app";

describe("GET /books", () => {
  it("responds with JSON containing a list of authors", async () => {
    const response = await request(app).get("/authors");
    expect(response.status).toBe(200);
  });

  it("book can be added via post", async () => {});
});
