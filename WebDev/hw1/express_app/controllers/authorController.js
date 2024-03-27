const { authors, Author } = require("../data");

exports.add_author = (req, res) => {
  const { full_name, bio, bday, p_genre } = req.body;
  const author = new Author(full_name, bio, bday, p_genre);
  console.log(req.body);
  authors.push(author);
  res.status(201).json({ message: "Author created successfully", author });
};

exports.list_authors = (req, res) => {
  res.status(200).json(authors);
};

exports.update_author_bio = (req, res) => {
  const { author_name } = req.params.id;
  const { bio } = req.body;
  const author = authors.find((author) => author.id === author_name);
  if (!author) {
    return res.status(404).json({ message: "Author not found" });
  }
  author.bio = bio;
  res.status(200).json({ message: "Author bio updated successfully", author });
};
