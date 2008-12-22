
all:
	yes 1 | (cd $(abs_srcdir) && $(PYTHON) setup.py build --build-base $(abs_builddir)/build)

install-exec-local:
	yes 1 | (cd $(abs_srcdir) && $(PYTHON) setup.py build --build-base $(abs_builddir)/build install --prefix=$(DESTDIR)$(prefix))

clean-local:
	rm -rf $(builddir)/build

uninstall-local:
	rm -rf $(DESTDIR)$(pythondir)/hcluster
	rm -f  $(DESTDIR)$(pythondir)/_hierarchy_wrap.so
	rm -f  $(DESTDIR)$(pythondir)/_distance_wrap.so
	rm -f  $(DESTDIR)$(pythondir)/hcluster-0.2.0-py2.5.egg-info

#
# Listing of all files in the hcluster distribution.
# TODO: remvoe the ests?  Or should we bother?
#
EXTRA_DIST = \
	setup.py \
	MANIFEST \
	PKG-INFO \
	TODO \
	THANKS \
	COPYING \
	hcluster/hierarchy.h \
	hcluster/distance.py \
	hcluster/common.h \
	hcluster/distance_wrap.c \
	hcluster/hierarchy.py \
	hcluster/hierarchy_wrap.c \
	hcluster/distance.h \
	hcluster/distance.c \
	hcluster/__init__.py \
	hcluster/hierarchy.c \
	hcluster/tests/pdist-cosine-ml.txt \
	hcluster/tests/pdist-euclidean-ml.txt \
	hcluster/tests/inconsistent-Q-single-2.txt \
	hcluster/tests/inconsistent-weighted-tdist-depth-4.txt \
	hcluster/tests/pdist-hamming-ml.txt \
	hcluster/tests/pdist-boolean-inp.txt \
	hcluster/tests/inconsistent-single-tdist-depth-0.txt \
	hcluster/tests/linkage-Q-single.txt \
	hcluster/tests/inconsistent-Q-single-5.txt \
	hcluster/tests/pdist-minkowski-5.8-ml-iris.txt \
	hcluster/tests/inconsistent-single-tdist-depth-3.txt \
	hcluster/tests/linkage-Q-median.txt \
	hcluster/tests/pdist-double-inp.txt \
	hcluster/tests/inconsistent-single-tdist-depth-5.txt \
	hcluster/tests/inconsistent-single-tdist-depth-1.txt \
	hcluster/tests/cdist-X1.txt \
	hcluster/tests/random-bool-data.txt \
	hcluster/tests/pdist-correlation-ml-iris.txt \
	hcluster/tests/pdist-correlation-ml.txt \
	hcluster/tests/inconsistent-complete-tdist-depth-2.txt \
	hcluster/tests/fclusterdata-maxclusts-3.txt \
	hcluster/tests/linkage-Q-centroid.txt \
	hcluster/tests/linkage-weighted-tdist.txt \
	hcluster/tests/inconsistent-Q-single-6.txt \
	hcluster/tests/linkage-Q-ward.txt \
	hcluster/tests/inconsistent-complete-tdist-depth-4.txt \
	hcluster/tests/linkage-Q-average.txt \
	hcluster/tests/inconsistent-complete-tdist-depth-3.txt \
	hcluster/tests/inconsistent-Q-single-4.txt \
	hcluster/tests/linkage-Q-complete.txt \
	hcluster/tests/pdist-seuclidean-ml.txt \
	hcluster/tests/inconsistent-weighted-tdist-depth-1.txt \
	hcluster/tests/pdist-chebychev-ml-iris.txt \
	hcluster/tests/fclusterdata-maxclusts-2.txt \
	hcluster/tests/inconsistent-single-tdist.txt \
	hcluster/tests/pdist-cityblock-ml.txt \
	hcluster/tests/cdist-X2.txt \
	hcluster/tests/inconsistent-Q-single-1.txt \
	hcluster/tests/inconsistent-single-tdist-depth-2.txt \
	hcluster/tests/inconsistent-complete-tdist-depth-1.txt \
	hcluster/tests/pdist-seuclidean-ml-iris.txt \
	hcluster/tests/Q-X.txt \
	hcluster/tests/inconsistent-weighted-tdist-depth-2.txt \
	hcluster/tests/linkage-X.txt \
	hcluster/tests/data.txt \
	hcluster/tests/inconsistent-Q-single-3.txt \
	hcluster/tests/pdist-cityblock-ml-iris.txt \
	hcluster/tests/test_hierarchy.py \
	hcluster/tests/pdist-minkowski-3.2-ml.txt \
	hcluster/tests/inconsistent-weighted-tdist-depth-3.txt \
	hcluster/tests/pdist-jaccard-ml.txt \
	hcluster/tests/linkage-average-tdist.txt \
	hcluster/tests/fclusterdata-maxclusts-4.txt \
	hcluster/tests/pdist-minkowski-3.2-ml-iris.txt \
	hcluster/tests/test_distance.py \
	hcluster/tests/linkage-Q-weighted.txt \
	hcluster/tests/linkage-single-tdist.txt \
	hcluster/tests/iris.txt \
	hcluster/tests/pdist-spearman-ml.txt \
	hcluster/tests/linkage-complete-tdist.txt \
	hcluster/tests/inconsistent-single-tdist-depth-4.txt \
	hcluster/tests/pdist-cosine-ml-iris.txt \
	hcluster/tests/pdist-euclidean-ml-iris.txt \
	hcluster/tests/pdist-chebychev-ml.txt

