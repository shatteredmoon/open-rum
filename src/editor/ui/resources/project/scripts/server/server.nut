SUCCESS <- 0;
FAILED  <- 1;

g_fMainTimer <- 0.0;

function onInit(serverTime)
{
    print ("Server initialization\n");

    g_fMainTimer = serverTime;

    // Seed the random number generator
    srand(time());

    return SUCCESS;
}


function onEditorInit(editorTime)
{
    print ("Editor: Initializing server scripts\n");
    return SUCCESS;
}


function onTick(serverTime)
{
    // print ("Server ticked\n");

    // Update the game timer
    g_fMainTimer = serverTime;
}


function onConnect(player, pawn)
{
   return player;
}
