// src/index.ts
import { MongoClient } from "mongodb";
import express from "express";
import reviewRouter from "./routes/reviews";

const app = express();
const port = 3000;

const mongo_url = "mongodb://localhost:27017";
const dbName = "my_db";

async function connectToMongo() {
  try {
    await dbClient.connect();
    console.log("Connected to MongoDB server");
  } catch (error) {
    console.error("Error connecting to MongoDB:", error);
  }
}

const dbClient = new MongoClient(mongo_url);
connectToMongo();
const database = dbClient.db(dbName);

app.use("/reviews", reviewRouter);
app.get("/", (req, res) => {
  res.send("Welcome to the reviews API with data persistance!");
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});

// Close db connection on process termination
process.on("SIGINT", async () => {
  try {
    await dbClient.close();
    console.log("Disconnected from MongoDB");
    process.exit(0);
  } catch (error) {
    console.error("Error closing MongoDB connection:", error);
    process.exit(1);
  }
});

export { database };
