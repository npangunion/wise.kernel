-- tx simple_query
CREATE PROCEDURE Usp.Hello
@result INT OUT
, @name VARCHAR
, @item1.id BIGINT
, @item1.price INT
, @is_ok BIT
, @options[0].add_hp INT
, @options[0].add_sp INT
, @options[1].add_hp INT
, @options[1].add_sp INT
, @options[2].add_hp INT
, @options[2].add_sp INT
, @options[3].add_hp INT
, @options[3].add_sp INT
, @options[4].add_hp INT
, @options[4].add_sp INT
, @items[0] INT
, @items[1] INT
, @items[2] INT
, @items[3] INT
, @items[4] INT
, @items[5] INT
, @items[6] INT
, @items[7] INT
, @items[8] INT
, @items[9] INT
AS 
BEGIN
-- rs_1 single row result set
rs_1.id INT
rs_1.sname NVARCHAR
-- rs_2 multiple row result set
rs_2.id TINYINT
rs_2.fv FLOAT
END -- tx simple_query


