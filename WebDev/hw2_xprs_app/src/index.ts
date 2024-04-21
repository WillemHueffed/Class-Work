// src/index.ts
// https://manage.auth0.com/dashboard/us/dev-7qw8fvj8e40n61ha/
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

const app = express();
// view engine setup
app.set("views", path.join(__dirname, "views"));
app.set("view engine", "pug");

app.use(logger("dev"));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, "public")));

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  next(createError(404));
});

// error handler
app.use(function(err: any, req: Request, res: Response, next: NextFunction) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get("env") === "development" ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render("error");
});

const port = 3001;
console.log(process.env.AUTH0_BASEURL);

const config = {
  authRequired: false,
  auth0Logout: true,
  secret: process.env.AUTH0_SECRET,
  baseURL: "http://localhost:3001",
  clientID: process.env.AUTH0_CLIENTID,
  issuerBaseURL: process.env.AUTH0_ISSUERBASEURL,
};
app.use(auth(config));
app.get("/", (req, res) => {
  res.send(req.oidc.isAuthenticated() ? "Logged in" : "Logged out");
});

const uri = process.env.MONGO_URI;
let mongoDB: Db;

const mongoClient = new MongoClient(uri as string, {
  pkFactory: { createPk: () => new UUID().toBinary() },
  serverApi: {
    version: ServerApiVersion.v1,
    strict: true,
    deprecationErrors: true,
  },
});
async function run() {
  try {
    // Connect the client to the server	(optional starting in v4.7)
    await mongoClient.connect();
    // Send a ping to confirm a successful connection
    await mongoClient.db("admin").command({ ping: 1 });
    console.log(
      "Pinged your deployment. You successfully connected to MongoDB!",
    );
    mongoDB = mongoClient.db("WebDev");
  } finally {
    // Ensures that the client will close when you finish/error
    //await mongoClient.close();
    //console.log("client closed");
  }
}
run().catch(console.dir);

app.use("/reviews", reviewRouter);
app.get("/", (req, res) => {
  res.send("Welcome to the reviews API with data persistance!");
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});

//const authConfig = {
//  domain: process.env.AUTH0_DOMAIN,
//  audience: process.env.AUTH0_AUDIENCE,
//};

export { mongoClient, mongoDB, app };
