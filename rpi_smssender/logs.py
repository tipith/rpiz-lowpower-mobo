from pathlib import Path
import logging, os, sys
import stat

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))


class LoggerWriter:
    """
    Fake file-like stream object that redirects writes to a logger instance.
    """
    def __init__(self, logger, log_level=logging.INFO):
        self.logger = logger
        self.log_level = log_level
        self.linebuf = ''

    def write(self, buf):
        for line in buf.rstrip().splitlines():
            self.logger.log(self.log_level, line.rstrip())


def setup_logging(log_subdir):
    log_file = Path(__location__, log_subdir, 'log.txt')
    if not log_file.parents[0].exists():
        log_file.parents[0].mkdir()

    #numeric_level = getattr(logging, loglevel.upper(), logging.DEBUG)

    fmt = logging.Formatter('%(levelname)8s %(asctime)s %(name)14s: %(message)s', datefmt="%Y-%m-%d %H:%M:%S")

    rootlog = logging.getLogger()
    rootlog.setLevel(logging.INFO)

    ch = logging.StreamHandler(sys.stdout)
    ch.setFormatter(fmt)
    rootlog.addHandler(ch)

    fh = logging.FileHandler(log_file)
    fh.setFormatter(fmt)
    rootlog.addHandler(fh)

    sys.stderr = LoggerWriter(rootlog)

    try:
        os.chmod(log_file, stat.S_IRUSR | stat.S_IWUSR | stat.S_IWGRP | stat.S_IWOTH)
    except OSError:
        pass