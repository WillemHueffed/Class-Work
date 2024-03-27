"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const router = express_1.default.Router();
router.get("/cool", (req, res, next) => {
    res.send("you're so cool!");
});
router.get("/", (req, res, next) => {
    res.send("respond with a resource");
});
exports.default = router;
