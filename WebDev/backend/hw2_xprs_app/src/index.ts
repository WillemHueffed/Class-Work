import { UUID } from "bson";
import logger from "morgan";
import createError from "http-errors";
import express, { Request, Response, NextFunction } from "express";
import { MongoClient, ServerApiVersion, Db } from "mongodb";
import reviewRouter from "./routes/reviews";
import { auth } from "express-openid-connect";
import dotenv from "dotenv";
import path from "path";
import cookieParser from "cookie-parser";
dotenv.config();

const config = {
  authRequired: false,
  auth0Logout: true,
  secret: process.env.AUTH0_SECRET,
  baseURL: "http://localhost:3001",
  clientID: process.env.AUTH0_CLIENTID,
  issuerBaseURL: process.env.AUTH0_ISSUERBASEURL,
};

const app = express();
// Set up Express middleware
app.set("views", path.join(__dirname, "views"));
app.set("view engine", "pug");
app.use(auth(config));
app.use(logger("dev"));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, "public")));

// Define Express routes
app.get("/", (req, res) => {
  res.send(req.oidc.isAuthenticated() ? "Logged in" : "Logged out");
});
app.use("/reviews", reviewRouter);
app.get("/", (req, res) => {
  res.send("Welcome to the reviews API with data persistance!");
});

// Error handler
app.use(function(err: any, req: Request, res: Response, next: NextFunction) {
  res.locals.message = err.message;
  res.locals.error = req.app.get("env") === "development" ? err : {};
  res.status(err.status || 500);
  res.render("error");
});

// MongoDB connection setup
const uri = process.env.MONGO_URI;
const mongoClient = new MongoClient(uri as string, {
  pkFactory: { createPk: () => new UUID().toBinary() },
  serverApi: {
    version: ServerApiVersion.v1,
    strict: true,
    deprecationErrors: true,
  },
});
let mongoDB: Db;

async function connectToMongoDB() {
  try {
    await mongoClient.connect();
    await mongoClient.db("admin").command({ ping: 1 });
    console.log(
      "Pinged your deployment. You successfully connected to MongoDB!",
    );
    mongoDB = mongoClient.db("WebDev");
    startServer();
  } catch (error) {
    console.error("Error connecting to MongoDB:", error);
    process.exit(1);
  }
}

function startServer() {
  const port = process.env.PORT || 3001;
  app.listen(port, () => {
    console.log(`Server is running on http://localhost:${port}`);
  });
}

// Start the MongoDB connection and server
connectToMongoDB();

export { mongoDB, app };
