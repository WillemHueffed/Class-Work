// src/index.ts
import { UUID } from "bson";
import { MongoClient, ServerApiVersion, Db } from "mongodb";
import express from "express";
import reviewRouter from "./routes/reviews";
import { auth } from "express-openid-connect";

const app = express();
const port = 3001;

const config = {
  authRequired: false,
  auth0Logout: true,
  secret: "X2ikQTGCxgGZf5_6XozZFL-iSi-DAQtAY48PO4xRsnZywboYZY9uk72bYrHUqois",
  baseURL: "http://localhost:3001",
  clientID: "TtqkDALr7skNQZykdGPp2e4UO03E7LNt",
  issuerBaseURL: "https://dev-7qw8fvj8e40n61ha.us.auth0.com",
};
app.use(auth(config));
app.get("/", (req, res) => {
  res.send(req.oidc.isAuthenticated() ? "Logged in" : "Logged out");
});

const uri =
  "mongodb+srv://whueffed:whueffed@webdev.nque75t.mongodb.net/?retryWrites=true&w=majority&appName=WebDev";
let mongoDB: Db;

const mongoClient = new MongoClient(uri, {
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
