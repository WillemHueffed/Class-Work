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
const editions: Edition[] = [];

export { Author, Book, Edition, authors, books, editions };
