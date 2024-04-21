import { v4 as uuidv4 } from "uuid";

class Author {
  name: string;
  bio: string;
  bday: string;
  genre: string;
  id: string;

  constructor(name: string, bio: string, bday: string, genre: string) {
    this.name = name;
    this.bio = bio;
    this.bday = bday;
    this.genre = genre;
    this.id = uuidv4();
  }
}

class Book {
  title: string;
  subtitle: string;
  org_pub_date: string;
  tags: string[];
  p_auth: Author;
  editions: Edition[];
  id: string;

  constructor(
    title: string,
    subtitle: string,
    org_pub_date: string,
    tags: string[],
    p_auth: Author,
  ) {
    this.title = title;
    this.subtitle = subtitle;
    this.org_pub_date = org_pub_date;
    this.tags = tags;
    this.p_auth = p_auth;
    this.id = uuidv4();
    this.editions = [];
  }
}

class Edition {
  ed_num: number;
  pub_date: string;
  id: string;

  constructor(ed_num: number, pub_date: string) {
    this.ed_num = ed_num;
    this.pub_date = pub_date;
    this.id = uuidv4();
  }
}

const authors: Author[] = [];
const books: Book[] = [];

for (let i = 1; i < 5; i++) {
  const author: Author = new Author(
    `name${i}`,
    `bio${i}`,
    `bday${i}`,
    `genre${i}`,
  );
  author.id = i.toString();
  const book: Book = new Book(
    `book${i}`,
    `sub${i}`,
    `pub${i}`,
    [`tag${i}`],
    author,
  );
  book.id = i.toString();
  authors.push(author);
  books.push(book);
}
for (let i = 0; i < books.length; i++) {
  for (let j = 1; j < 4; j++) {
    const edition: Edition = new Edition(j, `date${j}`);
    books[i].editions.push(edition);
  }
}

export { Author, Book, Edition, authors, books };
