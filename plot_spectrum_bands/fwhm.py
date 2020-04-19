import numpy as np

def FWHM(X,Y):
    m = max(Y) / 2
    d = Y - m
    indexes = np.where(d > 0)[0]
    return abs(X[indexes[-1]] - X[indexes[0]])

def FWHM_hlines(X,Y):
    y = max(Y) / 2
    d = Y - y
    indexes = np.where(d > 0)[0]
    return y, X[indexes[0]], X[indexes[-1]]

def CWLs(X,Y):
    y = max(Y) / 2
    d = Y - y
    indexes = np.where(d > 0)[0]
    return np.mean(X[indexes])