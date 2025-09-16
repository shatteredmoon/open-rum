
g_arrayKeyboardModes <- [];
g_eCurrentKeyboardMode <- 0;

g_tblStringCache <- {};


function CalculateListViewSize(ciListView, strFont, uiMaxRows)
{
    if (ciListView instanceof rumListView)
    {
        local uiTotalRows = ciListView.GetNumEntries();
        uiTotalRows = max(uiTotalRows, 1);

        local uiFontHeight = rumGetTextHeight(strFont);
        if (uiFontHeight > 0)
        {
            local uiTotalViewHeight = uiTotalRows * uiFontHeight;
            local uiRestrictedViewHeight = uiMaxRows * uiFontHeight;

            if (uiTotalViewHeight > uiRestrictedViewHeight)
            {
                ciListView.ShowScrollbar(true);
                ciListView.SetHeight(uiRestrictedViewHeight);
            }
            else
            {
                ciListView.ShowScrollbar(false);
                ciListView.SetHeight(uiTotalViewHeight);
            }
        }
        else
        {
            ciListView.SetHeight(16);
        }
    }
}


function GetString(str, ...)
{
    if (str in g_tblStringCache)
    {
        //print(str + " was cached\n");
        return g_tblStringCache[str];
    }

    //print(str + " was not cached\n");

    local lang_id = rumGetDefaultLanguage();
    local db_id = ::rumCLIENT_STRINGS_DB;

    switch(vargv.len())
    {
        case 2: db_id = vargv[1]; // fall through
        case 1: lang_id = vargv[0]; break
    }

    local loc = rumGetString(str, lang_id, db_id);
    g_tblStringCache[str] <- loc;

    return loc;
}


// Actual work is done here so that other function can reuse printing
function ShowString(str, ...)
{
    local color;

    switch(vargv.len())
    {
        case 1: color = vargv[0]; break;
        default: color = ""; break
    }

    local strOut = color + str;

    // Print the result to the client's game messages
    g_ciGameTextView.PushText(strOut);
}


class KeyboardMode
{
    // The keyboard mode to set
    mode = 0;

    // The function to call when this keyboard mode is popped
    callback = null;

    // Param to pass to callback function
    param = null;

    constructor(m, c, p)
    {
        // print("Keyboard mode callback type: " + typeof c + "\n");
        mode = m;
        callback = c;
        param = p;
    }
}


function PopKeyboardMode(bDoCallback, ...)
{
    if (g_arrayKeyboardModes.len() > 1)
    {
        // Get the current and previous keyboard modes
        local current = g_arrayKeyboardModes.pop();
        local previous = g_arrayKeyboardModes.top();
        if (current instanceof KeyboardMode && previous instanceof KeyboardMode)
        {
            // Switch current keyboard mode to the previous mode
            ::g_eCurrentKeyboardMode = previous.mode;

            // Get the callback function of the current mode
            if (bDoCallback)
            {
                local callback = current.callback;
                local param = current.param;
                if (callback)
                {
                    if (param != null)
                    {
                        // Perform the callback with params
                        switch(vargv.len())
                        {
                            case 1: callback.call(getroottable(), param, vargv[0]); break
                            case 2: callback.call(getroottable(), param, vargv[0], vargv[1]); break
                            case 3: callback.call(getroottable(), param, vargv[0], vargv[1], vargv[2]); break

                            case 0:
                            default: callback.call(getroottable(), param); break;
                        }
                    }
                    else
                    {
                        // Perform the callback without params
                        switch(vargv.len())
                        {
                            case 1: callback.call(getroottable(), vargv[0]); break
                            case 2: callback.call(getroottable(), vargv[0], vargv[1]); break
                            case 3: callback.call(getroottable(), vargv[0], vargv[1], vargv[2]); break

                            case 0:
                            default: callback.call(getroottable()); break;
                        }
                    }
                }
            }
        }
    }

    //print("POP - KB MODE SIZE: " + g_arrayKeyboardModes.len() + "\n");
}


function PushKeyboardMode(eMode, ...)
{
    if (eMode >= 0 && eMode < keyboardMode.NumModes)
    {
        local funcCallback = null;
        local varParam = null;

        if(vargv.len() > 0)
        {
            funcCallback = vargv[0];

            if (vargv.len() > 1)
            {
                varParam = vargv[1];
            }
        }

        local ciMode = KeyboardMode(eMode, funcCallback, varParam);
        if (ciMode instanceof KeyboardMode)
        {
            g_arrayKeyboardModes.push(ciMode)
            ::g_eCurrentKeyboardMode = eMode;
        }
    }

    //print("PUSH - KB MODE SIZE: " + g_arrayKeyboardModes.len() + "\n");
}
