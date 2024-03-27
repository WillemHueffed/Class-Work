"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.update_author_bio = exports.list_authors = exports.add_author = void 0;
const data_1 = require("../data");
const add_author = (req, res) => {
    const { full_name, bio, bday, p_genre } = req.body;
    const author = new data_1.Author(full_name, bio, bday, p_genre);
    console.log(req.body);
    data_1.authors.push(author);
    res.status(201).json({ message: "Author created successfully", author });
};
exports.add_author = add_author;
const list_authors = (req, res) => {
    res.status(200).json(data_1.authors);
};
exports.list_authors = list_authors;
const update_author_bio = (req, res) => {
    const { author_name } = req.params;
    const { bio } = req.body;
    const author = data_1.authors.find((author) => author.name === author_name);
    if (!author) {
        res.status(404).json({ message: "Author not found" });
        return;
    }
    author.bio = bio;
    res.status(200).json({ message: "Author bio updated successfully", author });
};
exports.update_author_bio = update_author_bio;
