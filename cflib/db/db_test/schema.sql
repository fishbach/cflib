CREATE TABLE config (
  key   text NOT NULL,
  value text,
  PRIMARY KEY (key)
);

-- REVISION bla 123
-- EXEC test1

ALTER TABLE config
  ADD value2 integer NOT NULL DEFAULT 0
;

-- EXEC test2
-- EXEC test3

ALTER TABLE config
  ADD value3 integer NOT NULL DEFAULT 0
;

-- EXEC test4
-- REVISION dudi hick

ALTER TABLE config
  ADD value4 integer NOT NULL DEFAULT 0
;

-- REVISION t1
-- REVISION t2

-- EXEC test5
