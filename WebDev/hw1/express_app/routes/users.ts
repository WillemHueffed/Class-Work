import express, { Request, Response, NextFunction } from "express";
const router = express.Router();

router.get("/cool", (req: Request, res: Response, next: NextFunction) => {
  res.send("you're so cool!");
});

router.get("/", (req: Request, res: Response, next: NextFunction) => {
  res.send("respond with a resource");
});

export default router;
