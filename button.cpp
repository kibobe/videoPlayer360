 #include "button.hpp"


Button::Button(Player *player)
    :m_player(player)
{
    m_action = 50;
    m_up = true;      
}

void Button::handleEvents(SDL_Event newEvent)
{
    m_event = newEvent;
    //Get the mouse offsets
    int x =m_event.button.x;
    int y =m_event.button.y;
    
     //If the user has Xed out the window
    if(m_event.type == SDL_QUIT)
    {
        cout << "exit" << endl;
        m_player->setPlayQuit(true);
    }
    //If a mouse button was pressed
    else if (m_event.type == SDL_MOUSEBUTTONDOWN)
    {
        //If the left mouse button was pressed
        if (m_event.button.button == SDL_BUTTON_LEFT)
        {
            m_beforeX = x;
            m_beforeY = y;
            m_up = false;
        }
    }
    //If a mouse button was released
    else if (m_event.type == SDL_MOUSEBUTTONUP)
    {
        //If the left mouse button was released
        if (m_event.button.button == SDL_BUTTON_LEFT )
        {
            m_up = true;
        }
    }
    else if (m_event.type == SDL_MOUSEMOTION)
    {
        if (m_event.button.button == SDL_BUTTON_LEFT )
        {
            if (!m_up)
            {
                m_player->lockPlayerMutex();
                if((y - m_beforeY) > m_action)
                {
                    m_beforeY = y;
                    double upDown = m_player->getUpDownAngle();
                    upDown += UD_ANGLE;

                    if (upDown >= PI/2)
                    {
                        m_player->setUpDownAngle(PI/2);
                    }
                    else
                    {
                        m_player->setUpDownAngle(upDown);
                    }
                }
                else if ((m_beforeY - y) > m_action) {
                    m_beforeY = y;
                    double upDown = m_player->getUpDownAngle();
                    upDown -= UD_ANGLE;
                    if (upDown <= -PI/2)
                    {
                        m_player->setUpDownAngle(-PI/2);
                    }
                    else
                    {
                        m_player->setUpDownAngle(upDown);
                    }
                 }

                if((x - m_beforeX) > m_action)
                {
                    m_beforeX = x;
                    int leftRight = m_player->getRotationAngle();
                    leftRight += RL_ANGLE;
                    m_player->setRotationAngle(leftRight);
                    m_player->setChangeLR(true);
                }
                else if((m_beforeX - x) > m_action)
                {
                    m_beforeX = x;
                    int leftRight = m_player->getRotationAngle();
                    leftRight -= RL_ANGLE;
                    m_player->setRotationAngle(leftRight);
                    m_player->setChangeLR(true);
                }
                m_player->unlockPlayerMutex();

            }
        }
    }
    
}   
