entity bot is
end entity;

architecture test of bot is
    signal x, y : natural;
begin

    p1: process is
    begin
        wait for 5 ns;
        x <= 5;
        wait;
    end process;

end architecture;

-------------------------------------------------------------------------------

entity ename1 is
end entity;

architecture test of ename1 is
    alias a is <<signal uut.x : natural>>;  -- Error
begin

    uut: entity work.bot;

end architecture;
