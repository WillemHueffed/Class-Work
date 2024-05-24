# 4/25
- https://www.youtube.com/watch?v=L8tWKqSMKUI&list=PL2q9pua8FpiUwVll2Alk8W_d3eN8IxJjO&ab_channel=CodetotheMoon
- https://actix.rs/docs/application
Just doing initial project setup and playing around with the start documentation
- example of mongodb with actix: https://github.com/actix/examples/tree/master/databases/mongodb

# 4/27
Spent like 1.5 hours getting the db connection setup. Still havn't fully tested it but its almost 10:30 and stuff compiled. Tried to follow
https://github.com/actix/examples/blob/master/databases/mongodb/src/main.rs but took a long ass time to modify it to my own stuff. Spent a lot
of time fighting with the type system. GPT was of little utility because of how niche the framework + db driver is. I've been kind of copying code
at a high level so there are some conecepts/documenation I need to look into...
- the doc! maco
- serde package
- future package

# 5/2
Found a guide for integrating auth0 with actix: https://auth0.com/blog/build-an-api-in-rust-with-jwt-authentication-using-actix-web/
Played around with the source code and looked at more technical documentation: https://docs.rs/mongodb/2.0.0/mongodb/struct.Cursor.html, https://www.mongodb.com/docs/drivers/rust/current/fundamentals/crud/read-operations/cursor/. Managed to get DB reads working.

Via a little GPT and checking the actix-web docs I got path extraction and request body extraction working. Also, the autocomplete descriptions from the rust LSP help a ton when
trying to work with the type system.

# 5/21
Spent like 5 hours marathoning the project to near completion. Everything works as intended. Just need to add in auth0. Things take so much longer to write in Rust. This is mostly
because I'm fighting with the type system. However I like Rusts functional features a lot. It makes dealing with return values really satisfying. As far as learning a new framework the concepts feel nearly 1:1 from Node/React. I havn't struggled with them at a high level at all. There are some nuances with the framework that are introduced from the framework being in rust that have tripped me up. This mainly has to do with functions returning types that are more complicated (encapsulates in Option<>/Result<>) than I'm expecting. I also tried really hard to serve static files from a directory and kept hitting 404s. The work around was to just serve each file individiually. I may be doing something wrong but googling it hasn't yielded any results so I'm pretty stumpted. This is definetly one of the major drawbacks of using a niche frameowork like actix.

# 5/22
I'm going crazy. Spent like 4 hours trying to get auth0 to work with actix. No luck. Found some random dude on youtube that manually implemented JWT auth tokens. Copied his source code and now we're chilling (after like another 4 hours of pain and suffering).
I've had to extend his code because he didn't provide signup/login/logout endpoints just a endpoint that returns a jwt and an endpoint that requires a jwt. 
Working on this project has made me aprecitate how convenient (braindead) JS is. This feels like writing assembly in comparison. As a side note I'm now uncomfortably familiar with how the JWT process works. Previously using auth0 I kinda just plugged and played. Going to work on salting/encrption and then all the functional requirements of the project should basically be done.

# 5/23
Dang near everything is done. Just need to refactor the hardcoded values to use the .env. All the static html pages to test the api are working with the exception of the patch
review one. I spent the better part of today getting the encrption up and running. That all works now so we're chilling.

# Analysis / Reflection

## Compare and Contrast
Express is a super steamlined expierience that has a high level of abstraction. Writing a simple API is like eating crayons. Actix isn't hard per se however because of being in 
Rust it quickly becomes complicated because of the language constructs. Namely the type system makes development much slower. I'm sure this would become less of an issue as I
become more expierienced but Rust will always be somewhat slower. Design wise they both have you delegate functions to certain predefined routes. I will say that implementing
middleware in express feels significantly easier than in rust. However I feel like I have much tighter control of what the server is doing when using Rust. I can specify hany
threads to run, how to manage the DB connection accross threads, etc...

## Scalability
Express is clearly scalable. Its used widely in industry. However I would argue that a team of *expierienced* rust developers could write more scalable code than a team of 
expierienced express devs. This is mainly due to the fact that because Rust is low level you can retain efficiency as you scale. Additionally, the type system means that if you 
change some communly used data structure and the consequences cascade throughout the codebase the compiler will catch these issues at compile time (and not runtime). While
you could argue typescript solves this issue for express 1. duck-typing is silly 2. rust's type system is stricter 3. the burden is on the programmer to define the types -> error
-> less scalable. 

## New project? 
For a quick hack project or something I needed to rapidely protype I'd go with express because its so low friction. Howver for a project I wanted to be built with "good bones"
I would go with Rust for the reasons mentioned above. I'd definetly recommend using rust/actix for a security intensive environment where you want tight control of whats
happenning.

## Personal Preference
I find writing react to be significantly less frustrating. However I think part of the reason this is, is because I'm still expieriencing the rust learning curve. I'm having to
learn completely new programming concepts (e.g. rust lifetimes) that make me feel like I'm a freshman in CS again. I wouldn't want to have to write anything super complicated 
in Actix/rust while I'm still learning it however I feel the language has a much better design than js/express. 

