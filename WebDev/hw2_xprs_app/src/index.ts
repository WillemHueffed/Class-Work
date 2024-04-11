// src/index.ts
import express from "express";

const app = express();
const port = 3000;

import reviewRouter from "./routes/reviews";

app.use("/reviews", reviewRouter);
app.get("/", (req, res) => {
  res.send("Hello, TypeScript with Express!");
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
