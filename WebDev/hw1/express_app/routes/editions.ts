import express, { Router } from "express";
import * as edition_controller from "../controllers/editionController";

const router: Router = express.Router({ mergeParams: true });

router.post("/", edition_controller.add_edition);
router.get("/", edition_controller.list_editions);
router.delete(":editionID", edition_controller.delete_edition);

export default router;
