const request = require("supertest");
const app = require("../app");

describe("GET /authors", () => {
  it("responds with JSON containing a list of authors", async () => {
    const response = await request(app).get("/authors");
    expect(response.status).toBe(200);
  });
});
