
from zope.interface import Interface


class ICarcassonneHandler(Interface):

    # called by client to server

    def do_login(username):
        """
        Call to login as somebody

        @return: C{True} or C{False}
        """

    def do_logout():
        """
        Call to logout
        """

    def do_enter(room_id):
        """
        Call to enter some game room

        @return: game information
        """

    def do_leave():
        """
        Call to leave the current game room
        """

    def do_chat(msg):
        """
        Call to chat;

        @return: C{True} or C{False}
        """

    def do_choose_color(color):
        """
        Call to choose a color

        @return: C{True} or C{False}
        """

    def do_ready():
        """
        Call to be ready for the game

        @return: C{True} or C{False}
        """

    def do_reveal_tile(x, y):
        """
        Reveal a tile at (x, y) on board if this is the current player;

        @return: tile index
        """

    def do_move_tile(x, y, rotation):
        """
        Move the revealed tile to specified coord and rotation;

        @return: {'fit': <boolean>, 'empty_terra': [<terra_idx>,]}
        @rtype: dict
        """

    def do_discard_tile():
        """
        Called when a player want to discard the last revealed tile

        @return: C{True} or C{False}
        """
    
    def do_confirm_discard_tile(yes):
        """
        Called when the other players confirm to the discard request
        """

    def do_put_meeple(terra_idx, coord_on_tile):
        """
        Called when the current player put a meeple on some terra

        @return: C{True} or C{False}
        """

    def do_pick_meeple():
        """
        Called when the current player doesn't want to put meeple on tile

        @return: C{True} or C{False}
        """
    
    def do_turn_end():
        """
        Called when the current player ends his turn

        @return: C{True} or C{False}
        """

    # called by server to client (notify)

    def notify_enter(username):
        """
        Notify clients that somebody has entered the room
        """

    def notify_leave(username):
        """
        Notify clients that somebody has leaved the room
        """
        
    def notify_chat(username, msg):
        """
        Notify clients that somebody has spoken some message
        """
        
    def notify_choose_color(username, color):
        """
        Notify clients that somebody has chosen some color
        """

    def notify_ready(username):
        """
        Notify clients that somebody has been ready for game
        """

    def notify_game_start():                                        # XXX
        """
        Notify clients that game is start
        """
    
    def notify_take_turn(username):
        """
        Notify clients that somebody will take turn
        """
        
    def notify_reveal_tile(item_id, tile_idx, x, y):
        """
        Notify clients that the curr player has revealed a tile on (x, y)
        """
        
    def notify_move_tile(item_id, x, y, rotation):
        """
        Notify clients that the curr player has moved a tile to (x, y) with rotation
        """

    def notify_requset_discard_tile(item_id):
        """
        Notify clients that the curr player want to discard the curr tile
        """

    def notify_confirm_discard_tile(item_id, username, yes):
        """
        Notify clients that some player confirm to the discard request
        """
        
    def notify_discard_tile_result(item_id, discarded):
        """
        Notify clients that the curr tile is discarded or not
        """

    def notify_put_meeple(item_id, terra_idx, coord_on_tile):
        """
        Notify clients that the curr player place a meeple on terra_idx on the tile
        """

    def notify_pick_meeple(item_id):
        """
        Notify clients that the curr player pick up the previous meeple put on tile
        """

    def notify_meeples_picked(item_to_score):
        """
        Notify clients that several items(meeples) are picked up with each some score

        @param item_to_score: dict of item_id to score
        """
        
    def notify_game_end(reason, ranking):
        """
        Notify clients that the game is ended with a ranking of usernames
        """
        
