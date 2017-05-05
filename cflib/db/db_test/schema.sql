CREATE TABLE config (
  key   text NOT NULL,
  value text,
  PRIMARY KEY (key)
);

-- REVISION bla 123

ALTER TABLE config
  ADD value2 int
;

-- EXEC update()
