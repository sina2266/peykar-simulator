#' Define an enumerated list
#'
#' @description
#'
#' Create an enumeration to be used instead of arbitrary values in code. In some
#' languages (C, C++, Python, etc.), enum (or enumeration) is a data type that
#' consists of integer constants and is ideal in contexts where a variable can
#' take on only one of a limited set of possible values (e.g. day of the week).
#' Since R programming language natively doesn't support enumeration, the
#' current function provides a way to create them using lists.
#'
#' @param enumValues A vector or a list of comma-separated constants to use for
#'   creating the enum. Optionally, these can be named constants.
#'
#' @return An enumerated list.
#'
#' @family enumeration-helpers
#'
#' @examples
#' # Without predefined values
#' Color <- enum(c("Red", "Blue", "Green"))
#' Color
#' myColor <- Color$Red
#' myColor
#'
#' # With predefined values
#' Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))
#' Symbol
#'
#' mySymbol <- Symbol$Diamond
#' mySymbol
#' @export

enum <- function(enumValues) {
  # if a vector, convert it to a list
  myEnum <- as.list(enumValues)

  # save names of a vector or a list
  enumNames <- names(myEnum)

  # check that none of the keys, if specified, are missing
  if (!is.null(enumNames) && any(enumNames == "")) {
    stop(messages$errorEnumNotAllNames)
  }

  # if no keys were specified, then use the values as keys themselves
  names(myEnum) <- enumNames %||% myEnum

  return(myEnum)
}

#' Get the key mapped to the given value in an `enum`
#'
#' @param enum The `enum` where the key-value pair is stored
#' @param value The value that is mapped to the `key`
#'
#' @return Key under which the value is stored. If the value is not in the enum,
#'   `NULL` is returned.
#'
#' @family enumeration-helpers
#'
#' @examples
#'
#' Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))
#' enumGetKey(Symbol, 1)
#' @export

enumGetKey <- function(enum, value) {
  output <- names(which(enum == value))

  if (length(output) == 0) {
    return(NULL)
  }

  return(output)
}

#' @rdname enumGetKey
#' @export

getEnumKey <- enumGetKey

#' Get enum values
#'
#' @description
#'
#' Return the value that is stored under the given key. If the key is not
#' present, an error is thrown.
#'
#' @param enum The `enum` that contains the key-value pair.
#' @param key The `key` under which the value is stored.
#'
#' @return Value that is assigned to `key`.
#'
#' @family enumeration-helpers
#'
#' @examples
#' Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))
#' enumGetValue(Symbol, "Diamond")
#' @export

enumGetValue <- function(enum, key) {
  if (!enumHasKey(key, enum)) {
    stop(messages$errorKeyNotInEnum(key, enum, deparse(substitute(enum))))
  }

  return(enum[[key]])
}

#' Get all keys of an enum
#'
#' @param enum `enum` containing the keys.
#'
#' @return List of `key` names.
#'
#' @family enumeration-helpers
#'
#' @examples
#' Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))
#' enumKeys(Symbol)
#' @export

enumKeys <- function(enum) {
  return(names(enum))
}

#' Check if an enum has a certain key.
#'
#' @param key Key to check for.
#' @param enum Enum where to look for the `key`.
#'
#' @return `TRUE` if a key-value pair for `key` exists, `FALSE` otherwise.
#'
#' @family enumeration-helpers
#'
#' @examples
#' Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))
#' enumHasKey("Diamond", Symbol)
#' enumHasKey("Square", Symbol)
#' @export

enumHasKey <- function(key, enum) {
  return(any(enumKeys(enum) == key))
}

#' Add a new key-value pairs to an `enum`
#'
#' @param keys Keys of the values to be added
#' @param values Values to be added
#' @param enum The enum to which the specified key-value pairs should be added.
#' **WARNING**: the original object is **not** modified!
#' @param overwrite If `TRUE` and a value with any of the given `keys` exists,
#'   it will be overwritten with the new value. Otherwise, an error is thrown.
#'   Default is `FALSE.`
#'
#' @return `Enum` with added key-value pair.
#'
#' @family enumeration-helpers
#'
#' @examples
#' myEnum <- enum(c(a = "b"))
#' myEnum <- enumPut("c", "d", myEnum)
#' myEnum <- enumPut(c("c", "d", "g"), c(12, 2, "a"), myEnum, overwrite = TRUE)
#' @export

enumPut <- function(keys, values, enum, overwrite = FALSE) {
  validateIsSameLength(keys, values)

  for (i in seq_along(keys)) {
    if (enumHasKey(keys[[i]], enum) && !overwrite) {
      stop(messages$errorKeyInEnumPresent(keys[[i]]))
    }

    enum[[keys[[i]]]] <- values[[i]]
  }

  return(enum)
}

#' Remove an entry from the enum.
#'
#' @param keys Key(s) of entries to be removed from the enum
#' @param enum Enum from which the entries to be removed
#' **WARNING**: the original object is not modified!
#'
#' @return Enum without the removed entries.
#'
#' @family enumeration-helpers
#'
#' @examples
#' Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))
#'
#' # either by key
#' enumRemove("Diamond", Symbol)
#'
#' # or by position
#' enumRemove(2L, Symbol)
#' @export

enumRemove <- function(keys, enum) {
  for (key in keys) {
    enum[[key]] <- NULL
  }

  return(enum)
}

#' Get the values stored in an enum
#'
#' @param enum `enum` containing the values
#'
#' @return List of values stored in the `enum`.
#'
#' @family enumeration-helpers
#'
#' @examples
#' Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))
#' enumValues(Symbol)
#' @export

enumValues <- function(enum) {
  return(unlist(enum, use.names = FALSE))
}
