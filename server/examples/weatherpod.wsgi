
import os
import ConfigParser

CONFIG_FILE = '/path/to/config.ini'
VIRTUALENV_PATH = '/path/to/venv'

activate_this = os.path.join(VIRTUALENV_PATH, 'bin/activate_this.py')
execfile(activate_this, dict(__file__=activate_this))

config = ConfigParser.ConfigParser()
config.read(CONFIG_FILE)

from weatherpod import server

application = server.Server(config).get_app()
