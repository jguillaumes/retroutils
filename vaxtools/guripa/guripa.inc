%ISSUCCESS: procedure (CODE) returns(character);
    %declare
        CODE  character,
        PTEXT character;
    %PTEXT = 'MOD(' || CODE ||',2)^=0';
    return(PTEXT);
%end;

%ISFAILURE: procedure (CODE) returns(character);
    %declare
        CODE  character,
        PTEXT character;
    %PTEXT = 'MOD(' || CODE ||',2)=0';
    return(PTEXT);
%end;

%activate ISSUCCESS,ISFAILURE;          

dcl gur_badint		bin fixed(31) globalref value,
    gur_runnin		bin fixed(31) globalref value,
    gur_notrun		bin fixed(31) globalref value,
    gur_nopriv		bin fixed(31) globalref value,
    gur_nombac		bin fixed(31) globalref value,
    gur_killed		bin fixed(31) globalref value,
    gur_tstamp		bin fixed(31) globalref value,
    gur_strtup		bin fixed(31) globalref value,
    gur_strted		bin fixed(31) globalref value,
    gur_exit		bin fixed(31) globalref value,
    gur_exitsn		bin fixed(31) globalref value,
    gur_status		bin fixed(31) globalref value,
    gur_badsyn		bin fixed(31) globalref value,
    gur_timout		bin fixed(31) globalref value;


