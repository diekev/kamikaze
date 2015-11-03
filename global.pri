# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2015 Kévin Dietrich.
# All rights reserved.
#
# ***** END GPL LICENSE BLOCK *****

CONFIG += c++11
CONFIG += no_keywords

QMAKE_CXXFLAGS += -O3 -msse -msse2 -msse3

QMAKE_CXXFLAGS_DEBUG += -g -Wall -Og -Wno-error=unused-function \
	-Wextra -Wno-missing-field-initializers -Wno-sign-compare -Wno-type-limits  \
	-Wno-unknown-pragmas -Wno-unused-parameter -Wno-ignored-qualifiers          \
	-Wmissing-format-attribute -Wno-delete-non-virtual-dtor                     \
	-Wsizeof-pointer-memaccess -Wformat=2 -Wno-format-nonliteral -Wno-format-y2k\
	-fstrict-overflow -Wstrict-overflow=2 -Wno-div-by-zero -Wwrite-strings      \
	-Wlogical-op -Wundef -DDEBUG_THREADS -Wnonnull -Wstrict-aliasing=2          \
	-fno-omit-frame-pointer -Wno-error=unused-result -Wno-error=clobbered       \
	-fstack-protector-all --param=ssp-buffer-size=4 -Wno-maybe-uninitialized    \
	-Wunused-macros -Wmissing-include-dirs -Wuninitialized -Winit-self          \
	-Wtype-limits -fno-common -fno-nonansi-builtins -Wformat-extra-args         \
	-Wno-error=unused-local-typedefs -DWARN_PEDANTIC -Winit-self -Wdate-time    \
	-Warray-bounds -Werror -fdiagnostics-color=always -fsanitize=address

CONFIG(debug, release|debug) {
	QMAKE_LFLAGS += -fsanitize=address
}

DEFINES += DWREAL_IS_DOUBLE=0
