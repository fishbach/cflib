CREATE TABLE config (
  key   text NOT NULL,
  value text,
  PRIMARY KEY (key)
);

-- REVISION bla 123
-- EXEC test1()

ALTER TABLE config
  ADD value2 int
;

-- EXEC test2()
-- EXEC test3()

ALTER TABLE config
  ADD value3 int
;

-- EXEC test4 ()
-- REVISION dudi hick

ALTER TABLE config
  ADD value4 int
;

-- REVISION t1
-- REVISION t2

-- EXEC test5()
