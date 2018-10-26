# Contributing #

The Cortex team always welcomes quality contributions and issue reports. The following guidelines exist to maintain a consistent quality for the project's history, and facilitate the code adoption and correction processes. The goal is for everyone to assist our project maintainers by delivering the necessary information for them to make informed decisions and reduce as much confusion and guesswork as possible. Adhering to these guidelines will also benefit you, as it will reduce the time it takes for your contribution to be merged or your issue to be resolved.

Feel free to join the discussion on the [Cortex developer group](https://groups.google.com/group/cortexdev). If you are new to Cortex development, we recommend browsing the latest discussions to develop a sense for the current priorities and familiarize yourself with the development flow.


## Reporting Bugs ##

If you discover behaviour in Cortex that you suspect is a bug, first:

1. Check to see whether it is a known bug on our [Issues page](https://github.com/ImageEngine/cortex/issues).
2. Check the [Cortex developer group](https://groups.google.com/group/cortexdev) to see whether others have already discussed it.

Once you have determined that the behaviour you have noticed is a bug, [create a new Issue](https://github.com/ImageEngine/cortex/issues/new) for it. Make sure to fill out the Issue template on GitHub.


## Contributing Code ##

### Pull Requests ###

If you have a fork of Cortex and have made improvements, and you would like to see them merged with the main project, you can create a new [Pull Request](https://github.com/ImageEngine/cortex/pulls) (PR). Make sure to fill out the PR template on GitHub.

> **Note:** If you are developing a _separate_ project based on Cortex, you have no obligation to merge your improvements to the main Cortex project – you can manage your project independently at your own discretion.

For small bug fixes and code cleanup, feel free to make a PR without consulting a project maintainer.

If you are planning on making a significant change, such as a new feature, a refactorization, or a rewrite, please discuss your plans first with the project maintainers on the [Cortex developer group](https://groups.google.com/group/cortexdev). This will help us all avoid duplicated effort and ensure that your ideas fit with the direction of the project. It's always a good idea to ask anyway, as our developers will be happy to suggest implementations and strategy.

If your PR adds a new feature, it must come with a unit test for the feature. Tested code is good code. We like proof that your code works!

All PRs are reviewed and approved by a project maintainer before being merged.


### Commits ###

Each commit in your PR must perform one logically distinct set of changes. It must also have an accompanying useful message that gives everyone a good idea of what you've done.

We have several message best practices, which, if followed, result in a succinct, informative commit history that can be displayed natively in a variety of protocols and applications, such as email and IDEs. The goal is for anyone to be able to look through the commit log on its own and have a reasonably detailed idea of what was changed and why.


#### Commit Message Best Practices ####

- Each line of the message should be 72 characters or less.
- The first line's message should start with the name of the module or area of the project being affected, followed by a space, a colon, another space, and finally by a _general_ description. Example: `Interface : Add MyButton`.
- If the message is multi-line, the second line should be blank, to preserve formatting.
- If the commit makes several small but important changes, list them line-by-line, with each line starting with a hyphen followed by a space, followed by a description of the change.


#### Example Commits for a New Feature ####

In the following example, a contributor has added a new button with custom functionality and UI to the main menu of an application.

------

```
Interface : Add MyButton

Button to add my new function, which does x. Uses similar  
implementation to a standard MenuButton, with a workaround to prevent  
the onClick event.
```

Files changed:
- MyButton.h
- MyButton.cpp
- Interface.cpp

------

```
GUI : Add MyButton to main window
```

Files changed:
- MyButton.py
- GUI.py


------

```
Resources : Add graphics for MyButton

- MyButtonDot.svg : red dot
- MyButtonCursor.svg : cursor for dragging button
- MyButtonBorder.svg : black border
- MyButtonGradient.svg : grey-to-white gradient for background
```

Files changed:
- MyButtonDot.svg
- MyButtonCursor.svg
- MyButtonBorder.svg
- MyButtonGradient.svg


------

```
Tests : Add unit test for MyButton
```

Files changed:
- MyButtonTest.py
