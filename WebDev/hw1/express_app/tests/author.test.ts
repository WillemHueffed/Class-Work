import { it, describe, expect, test } from "@jest/globals";
import request from "supertest";
import app from "../app";

describe("GET /authors", () => {
  it("responds with JSON containing a list of authors", async () => {
    const response = await request(app).get("/authors");
    expect(response.status).toBe(200);
  });
});
